#include "SESSION.h"
#include "DataBaseManager.h"
#include "AMBIT.h"

OVER_EXP::OVER_EXP()
{
	_wsabuf.len = BUF_SIZE;
	_wsabuf.buf = _send_buf;
	_comp_type = OP_RECV;
	ZeroMemory(&_over, sizeof(_over));
};

OVER_EXP::OVER_EXP(unsigned char* packet)
{
	_wsabuf.len = packet[0];
	_wsabuf.buf = _send_buf;
	ZeroMemory(&_over, sizeof(_over));

	_comp_type = OP_SEND;
	memcpy(_send_buf, packet, packet[0]);
};

void SESSION::send_move_packet(int c_id, int client_time)
{
	SC_MOVE_OBJECT_PACKET p;
	p.id = c_id;
	p.size = sizeof(SC_MOVE_OBJECT_PACKET);
	p.type = SC_MOVE_OBJECT;
	p.x = clients[c_id].X();
	p.y = clients[c_id].Y();
	p.client_time = client_time;
	do_send(&p);
}

void SESSION::send_remove_packet(int c_id)
{
	SC_REMOVE_OBJECT_PACKET p;
	p.id = c_id;
	p.size = sizeof(p);
	p.type = SC_REMOVE_OBJECT;
	do_send(&p);
}

void SESSION::send_chat_packet(int c_id, const char* mess)
{
	SC_CHAT_PACKET p;
	p.id = c_id;
	p.size = sizeof(SC_CHAT_PACKET) - sizeof(p.mess) + strlen(mess) + 1;
	p.type = SC_CHAT;

	//System Message
	if (c_id < 0)
	{
		p.chat_type = 3;
		strcpy_s(p.name, "_SYSTEM");
		strcpy_s(p.mess, mess);
		printf(" : %s", p.mess);
	}
	else
	{
		p.chat_type = 0;
		strcpy_s(p.name, clients[c_id].NAME());
		strcpy_s(p.mess, mess);
		printf("chat : %s\n", p.mess);
	}

	do_send(&p);
}

void SESSION::send_stat_change_packet(int c_id)
{
	SC_STAT_CHANGE_PACKET p;
	p.id = c_id;
	p.size = sizeof(SC_STAT_CHANGE_PACKET);
	p.type = SC_STAT_CHANGE;

	p.level = clients[c_id].getData().level;
	p.exp = clients[c_id].getData().EXP;
	p.hp = clients[c_id].getData().HP;
	p.hpmax = clients[c_id].getData().MaxHP;
	p.mp = clients[c_id].getData().MP;
	p.mpmax = clients[c_id].getData().MaxMP;

	do_send(&p);
}


SESSION::SESSION()
{
	//_id = -1;
	_socket = 0;
	//x = y = 0;
	sectorX = sectorY = 0;
	//_name[0] = 0;
	_s_state = ST_FREE;
	_prev_remain = 0;
}

SESSION::~SESSION()
{
}

void SESSION::do_recv()
{
	DWORD recv_flag = 0;
	memset(&_recv_over._over, 0, sizeof(_recv_over._over));
	_recv_over._wsabuf.len = BUF_SIZE - _prev_remain;
	_recv_over._wsabuf.buf = _recv_over._send_buf + _prev_remain;

	int ret = WSARecv(_socket, &_recv_over._wsabuf, 1, 0, &recv_flag, &_recv_over._over, 0);

}

void SESSION::do_send(void* packet)
{
	OVER_EXP* sdata = new OVER_EXP{ reinterpret_cast<unsigned char*>(packet) };
	WSASend(_socket, &sdata->_wsabuf, 1, 0, 0, &sdata->_over, 0);
}

void SESSION::send_login_info_packet()
{
	if (data.isValidLogin)
	{
		SC_LOGIN_OK_PACKET p;
		p.id = data.sc_id;
		strcpy(p.name, data.name);
		p.size = sizeof(SC_LOGIN_OK_PACKET) - sizeof(p.name) + strlen(p.name) + 1;
		p.type = SC_LOGIN_OK;
		p.level = data.level;
		p.HP = data.HP;
		p.MaxHP = data.MaxHP;
		p.MP = data.MP;
		p.MaxMP = data.MaxMP;
		p.EXP = data.EXP;
		p.x = data.x;
		p.y = data.y;
		do_send(&p);
	}
	else
	{
		send_login_fail_packet(0);
	}
}

void SESSION::send_login_fail_packet(int reason)
{
	SC_LOGIN_FAIL_PACKET p;
	p.size = sizeof(SC_LOGIN_FAIL_PACKET);
	p.type = SC_LOGIN_FAIL;
	p.reason = reason;
	do_send(&p);
}

void SESSION::send_put_packet(int c_id)
{
	//printf("ADD player : %d", c_id);
	SC_ADD_OBJECT_PACKET put_packet;
	put_packet.size = sizeof(put_packet);
	put_packet.type = SC_ADD_OBJECT;
	put_packet.id = c_id;
	if (clients[c_id].getData().isPlayer)
	{
		put_packet.race = 0;
	}
	else
	{
		put_packet.race = clients[c_id].getData().level + 1;
	}
	//strcpy(put_packet.name, data.name);
	put_packet.x = clients[c_id].X();
	put_packet.y = clients[c_id].Y();

	strcpy_s(put_packet.name, clients[c_id].NAME());
	put_packet.level = clients[c_id].getData().level;
	put_packet.hp = clients[c_id].getData().HP;
	put_packet.hpmax = clients[c_id].getData().MaxHP;
	put_packet.mp = clients[c_id].getData().MP;
	put_packet.mpmax = clients[c_id].getData().MaxMP;
	do_send(&put_packet);
}

void SESSION::setXY(short _x, short _y)
{
	sectorX = X() / SECTOR_WIDTH;
	sectorY = Y() / SECTOR_HEIGHT;

	if (sectorX != _x / SECTOR_WIDTH || sectorY != _y / SECTOR_HEIGHT)
	{
		//erase가 thread unsafe이므로 그냥 false로 변환
		sectors[sectorX][sectorY][ID()] = false;
		//false가 너무 많이 쌓이면 느려지므로 이후 주기적으로 지워줘야함 (how?)

		sectorX = _x / SECTOR_WIDTH;
		sectorY = _y / SECTOR_HEIGHT;

		sectors[sectorX][sectorY][ID()] = true;
	}

	s_Map::s_movemap[data.x][data.y] = false;
	data.x = _x;
	data.y = _y;
	s_Map::s_movemap[data.x][data.y] = true;

	if (sectors[sectorX][sectorY][ID()] == false)
	{
		//printf("%d sector f->t",_id);
		sectors[sectorX][sectorY][ID()] = true;
	}

}

short SESSION::X()
{
	return data.x;
}

short SESSION::Y()
{
	return data.y;
}

int SESSION::ID()
{
	return data.sc_id;
}
void SESSION::ID(int a)
{
	data.sc_id = a;
}

void SESSION::setData(LoginData& d)
{
	data = d;
	setXY(d.x, d.y);
}

LoginData& SESSION::getData()
{
	return data;
}

char* SESSION::NAME()
{
	return data.name;
}

void SESSION::setDamage(int& damage)
{
	data.HP -= damage;
	if (!data.isPlayer)
	{
		ll.lock();
		lua_getglobal(clients[npc_id].L, "set_state");
		lua_pushnumber(clients[npc_id].L, 11);
		int error = lua_pcall(clients[npc_id].L, 1, 0, 0);
		if (error) {
			cout << "Error:" << lua_tostring(clients[npc_id].L, -1);
			lua_pop(clients[npc_id].L, 1);
		}
		ll.unlock();
	}

	if (data.HP <= 0)
	{
		damage = -1;
		data.HP = 0;
		adaptDeath();
	}
}

void SESSION::adaptDeath()
{
	if (data.isPlayer)
	{
		data.EXP = data.EXP / 2;
		data.HP = data.MaxHP;
		send_stat_change_packet(data.sc_id);

		CS_MOVE_PACKET p;
		p.direction = -1;
		p.client_time = 0;
		p.size = sizeof(CS_MOVE_PACKET);
		p.type = CS_MOVE;

		OVER_EXP* sdata = new OVER_EXP{ reinterpret_cast<unsigned char*>(reinterpret_cast<void *>(&p)) };
		sdata->_comp_type = OP_RECV;
		PostQueuedCompletionStatus(g_h_iocp, 0, data.sc_id, reinterpret_cast<LPOVERLAPPED>(&sdata));
	}
	else
	{
		//부활 티켓 걸어놓기
		if (_s_state == ST_INGAME)
		{
			SESSION_STATE sst = ST_INGAME;
			if (atomic_compare_exchange_weak(&_s_state, &sst, ST_NPC_DEAD))
			{
				data.HP = data.MaxHP;
				HeartManager::add_timer(npc_id, 30000, EV_RESURRECTION, npc_id);
			}
		}
	}
}

bool SESSION::appendEXP(int exp)
{
	data.EXP += exp;
	if (data.EXP >= 100 + 100 * data.level)
	{
		data.EXP -= 100 + 100 * data.level;
		data.level++;
		data.MaxHP += 50;
		data.MaxMP += 10;
		data.HP = data.MaxHP;
		data.MP = data.MaxMP;

		return true;
	}
	return false;
}

void SESSION::autoHeal()
{
	data.HP += data.MaxHP / 10;
	if (data.HP > data.MaxHP)
		data.HP = data.MaxHP;
	data.MP += data.MaxMP / 10;
	if (data.MP > data.MaxMP)
		data.MP = data.MaxMP;
}