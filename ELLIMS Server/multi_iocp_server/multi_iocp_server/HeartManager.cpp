#include "HeartManager.h"

array<chrono::system_clock::time_point, NUM_NPC> HeartManager::next_move_times;
priority_queue<TIMER_EVENT> HeartManager::timer_queue;
mutex HeartManager::timer_l;

void HeartManager::initialize_npc()
{
    printf("NPC Setting Start");
    for (int i = 0; i < NUM_NPC; ++i)
    {
        LoginData ld;
        sprintf_s(ld.name, "M-%d", i);
        int npc_id = i + MAX_USER;
        ld.sc_id = npc_id;
        clients[npc_id].setData(ld);
        clients[npc_id]._s_state = ST_INGAME;

        HeartManager::add_timer(npc_id, 1000, TIMER_EVENT_TYPE::EV_MOVE, 0);
    }
    printf("NPC Setting End");
}

void HeartManager::add_timer(int obj_id, int act_time, TIMER_EVENT_TYPE e_type, int target_id)
{
    using namespace chrono;
    TIMER_EVENT ev;
    ev.act_time = system_clock::now() + milliseconds(act_time);
    ev.object_id = obj_id;
    ev.ev = e_type;
    ev.target_id = target_id;
    timer_queue.push(ev);
}


void HeartManager::move_npc(int npc_id)
{
    short x = clients[npc_id].X();
    short y = clients[npc_id].Y();
    unordered_set<int> old_vl;
    for (int i = 0; i < MAX_USER; ++i) {
        if (clients[i]._s_state != ST_INGAME) continue;
        if (distance(npc_id, i) <= RANGE) old_vl.insert(i);
    }
    switch (rand() % 4) {
    case 0: if (y > 0) y--; break;
    case 1: if (y < W_HEIGHT - 1) y++; break;
    case 2: if (x > 0) x--; break;
    case 3: if (x < W_WIDTH - 1) x++; break;
    }

    clients[npc_id].setXY(x,y);

    //나중에 섹터처리좀

    unordered_set<int> new_vl;
    for (int i = 0; i < MAX_USER; ++i) {
        if (clients[i]._s_state != ST_INGAME) continue;
        if (distance(npc_id, i) <= RANGE) new_vl.insert(i);
    }

    for (auto p_id : new_vl) {
        clients[p_id].vl.lock();
        if (0 == clients[p_id].view_list.count(npc_id)) {
            clients[p_id].view_list.insert(npc_id);
            clients[p_id].vl.unlock();
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
}

void HeartManager::ai_thread()
{
    while (true)
    {
        while (timer_queue.top().act_time > chrono::system_clock::now())
        {
            SleepEx(1, TRUE);
        }
        TIMER_EVENT t = timer_queue.top();
        timer_queue.pop();

        switch (t.ev)
        {
        case TIMER_EVENT_TYPE::EV_ATTACK:
        {
            
        }
        break;
        case TIMER_EVENT_TYPE::EV_MOVE:

            OVER_AI over;
            over._timer_type = EV_MOVE;
            over._comp_type = OP_AI;
            PostQueuedCompletionStatus(g_h_iocp, 0, t.object_id, reinterpret_cast<LPOVERLAPPED>(&over));

            break;
        case TIMER_EVENT_TYPE::EV_HEAL:
            break;

        }
    }
}
