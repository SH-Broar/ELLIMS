#include "HeartManager.h"
#include "AMBIT.h"

array<chrono::system_clock::time_point, NUM_NPC> HeartManager::next_move_times;
priority_queue<TIMER_EVENT> HeartManager::timer_queue;
mutex HeartManager::timer_l;

void HeartManager::initialize_npc()
{
	printf("NPC Setting Start");
	for (int i = 0; i < NUM_NPC; ++i)
	{
		LoginData ld;
		ld.x = rand() % W_WIDTH;
		ld.y = rand() % W_HEIGHT;
		sprintf_s(ld.name, "M-%d", i);
		int npc_id = i + MAX_USER;
		clients[npc_id].npc_id = npc_id;
		ld.sc_id = npc_id;

		clients[npc_id].setData(ld);

		clients[npc_id].L = luaL_newstate();
		luaL_openlibs(clients[npc_id].L);
		luaL_loadfile(clients[npc_id].L, "monsterScript.lua");
		int error = lua_pcall(clients[npc_id].L, 0, 0, 0);

		if (error) {
			cout << "Error:" << lua_tostring(clients[npc_id].L, -1);
			lua_pop(clients[npc_id].L, 1);
		}

		lua_register(clients[npc_id].L, "astar", A_Star_Pathfinding);
		lua_register(clients[npc_id].L, "roam", Roaming);

		//나중에 lua에게 일임
		lua_getglobal(clients[npc_id].L, "set_object_id");
		lua_pushnumber(clients[npc_id].L, npc_id);
		lua_pushnumber(clients[npc_id].L, 1);
		lua_pushnumber(clients[npc_id].L, ld.x);
		lua_pushnumber(clients[npc_id].L, ld.y);
		lua_pushnumber(clients[npc_id].L, rand()%5);
		lua_pushnumber(clients[npc_id].L, rand()%20);
		//lua_pushstring(clients[npc_id].L, "monster");
		lua_pcall(clients[npc_id].L, 6, 0, 0);


		clients[npc_id]._s_state = ST_NPC_SLEEP;
	}
	printf("NPC Setting End");
}

void HeartManager::add_timer(int obj_id, int act_time, TIMER_EVENT_TYPE e_type, int target_id)
{
	using namespace chrono;
	TIMER_EVENT ev;
	ev.act_time = system_clock::now() + milliseconds(1000);
	ev.object_id = obj_id;
	ev.ev = e_type;
	ev.target_id = target_id;
	timer_queue.push(ev);
}


void HeartManager::move_npc(int npc_id, int target_id)
{
	int x = clients[npc_id].X();
	int y = clients[npc_id].Y();
	unordered_set<int> old_vl;
	for (int i = 0; i < MAX_USER; ++i) {
		if (clients[i]._s_state != ST_INGAME) continue;
		if (distance_cell(npc_id, i) <= RANGE) old_vl.insert(i);
	}


	//lua에 길찾기 넘기기
	clients[npc_id].ll.lock();
	lua_getglobal(clients[npc_id].L, "npc_move");
	lua_pushnumber(clients[npc_id].L, x);
	lua_pushnumber(clients[npc_id].L, y);
	lua_pushnumber(clients[npc_id].L, clients[target_id].X());
	lua_pushnumber(clients[npc_id].L, clients[target_id].Y());
	int error = lua_pcall(clients[npc_id].L, 4, 2, 0);
	if (error) {
		cout << "Error:" << lua_tostring(clients[npc_id].L, -1);
		lua_pop(clients[npc_id].L, 1);
	}

	x = lua_tonumber(clients[npc_id].L, -2);
	y = lua_tonumber(clients[npc_id].L, -1);
	lua_pop(clients[npc_id].L, 2);
	clients[npc_id].ll.unlock();

	if (x > 0 && x < W_WIDTH - 1 && y > 0 && y < W_HEIGHT - 1 && s_Map::canMove(x, y))
	{
		clients[npc_id].setXY(x, y);
	}

	//나중에 섹터처리좀

	unordered_set<int> new_vl;
	bool is_player_exist = false;
	int targ = npc_id;
	for (int i = 0; i < MAX_USER; ++i) {
		if (clients[i]._s_state != ST_INGAME) continue;
		if (distance_cell(npc_id, i) <= RANGE)
		{
			new_vl.insert(i);
			is_player_exist |= true;
			targ = i;
		}
	}

	for (auto p_id : new_vl) {
		clients[p_id].vl.lock();
		if (0 == clients[p_id].view_list.count(npc_id)) {
			clients[p_id].view_list.insert(npc_id);
			clients[p_id].vl.unlock();
			//printf("후보 1");
			clients[p_id].send_put_packet(npc_id);
		}
		else {
			clients[p_id].vl.unlock();
			clients[p_id].send_move_packet(npc_id, 0);
		}
	}
	for (auto p_id : old_vl) {
		if (0 == new_vl.count(p_id)) {
			clients[p_id].vl.lock();
			if (clients[p_id].view_list.count(npc_id) == 1) {
				clients[p_id].view_list.erase(npc_id);
				clients[p_id].vl.unlock();
				clients[p_id].send_remove_packet(npc_id);
			}
			else
				clients[p_id].vl.unlock();
		}
	}
	if (is_player_exist)
	{
		add_timer(npc_id, 1000, EV_MOVE, targ);
	}
	else
	{
		clients[npc_id]._s_state = ST_NPC_SLEEP;
	}
}

int HeartManager::A_Star_Pathfinding(lua_State* L)
{
	int x = lua_tonumber(L, -4);
	int y = lua_tonumber(L, -3);
	int tx = lua_tonumber(L, -2);
	int ty = lua_tonumber(L, -1);
	lua_pop(L, 4);

	// astar
	//

	lua_pushnumber(L, rand() % 4 + 1);
	return 1;

}

int HeartManager::Roaming(lua_State* L)
{
	//printf("  RAND  ");
	int x = lua_tonumber(L, -2);
	int y = lua_tonumber(L, -1);
	lua_pop(L, 2);


	lua_pushnumber(L, rand() % 4 + 1);
	return 1;
}

void HeartManager::ai_thread()
{
	while (true)
	{
		if (timer_queue.empty())
		{
			//printf("empty\n");
			SleepEx(1, TRUE);
			continue;
		}
		TIMER_EVENT t = timer_queue.top();
		while (t.act_time > chrono::system_clock::now())
		{
			//auto p = std::chrono::duration_cast<std::chrono::milliseconds>(timer_queue.top().act_time - chrono::system_clock::now());
			//std::cout << p <<endl;
			SleepEx(1, TRUE);
		}
		timer_queue.pop();

		switch (t.ev)
		{
		case TIMER_EVENT_TYPE::EV_ATTACK:
		{

		}
		break;
		case TIMER_EVENT_TYPE::EV_MOVE:
		{
			//printf("EV_MOVE");
			OVER_AI over;
			over.object_id = t.object_id;
			over.target_id = t.target_id;
			over._timer_type = EV_MOVE;
			over._comp_type = OP_AI;
			PostQueuedCompletionStatus(g_h_iocp, 0, t.object_id, reinterpret_cast<LPOVERLAPPED>(&over));
		}
		break;
		case TIMER_EVENT_TYPE::EV_HEAL:
			break;

		}
		SleepEx(1, TRUE);
	}
}
