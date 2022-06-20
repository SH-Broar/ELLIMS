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
	put_packet.race = clients[c_id].getData().race;
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
		//erase�� thread unsafe�̹Ƿ� �׳� false�� ��ȯ
		sectors[sectorX][sectorY][ID()] = false;
		//false�� �ʹ� ���� ���̸� �������Ƿ� ���� �ֱ������� ��������� (how?)

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

		vl.lock();
		unordered_set<int> r_view_list = view_list;
		view_list.clear();
		vl.unlock();

		for (auto& v : r_view_list)
		{
			send_remove_packet(v);
		}
		
		setXY(W_WIDTH / 2, W_HEIGHT / 2);

		for (int i = 0; i < 3; ++i)
		{
			for (int h = 0; h < 3; ++h)
			{
				if (sectorX - 1 + i < 0 ||
					sectorX - 1 + i >= W_WIDTH / SECTOR_WIDTH ||
					sectorY - 1 + h < 0 ||
					sectorY - 1 + h >= W_HEIGHT / SECTOR_HEIGHT)
					continue;

				for (auto& pln : sectors[sectorX - 1 + i][sectorY - 1 + h])
				{
					if (pln.second == false)
						continue;
					if (pln.first == ID())
					{
						send_move_packet(ID(), 0);
						continue;
					}

					auto& pl = clients[pln.first];

					//���� ���� NPC �����
					if (pl._s_state == ST_NPC_SLEEP)
					{
						SESSION_STATE sst = ST_NPC_SLEEP;
						if (atomic_compare_exchange_weak(&(pl._s_state), &sst, ST_INGAME))
						{
							HeartManager::add_timer(pl.npc_id, 1000, EV_MOVE, ID());
						}
					}

					if (pl.ID() >= MAX_USER) continue;
					// lock_guard<mutex> aa{pl._sl};	//���� ���

					if (pl._s_state != ST_INGAME)continue;

					//near�� ��� ��ü�� ����?
					if (distance_cell(ID(), pl.ID()) <= RANGE)
					{
						vl.lock();
						//������
						if (!view_list.contains(pl.ID()))
						{
							view_list.insert(pl.ID());
							vl.unlock();

							//put
							send_put_packet(pl.ID());


							//���� ������� ������
							pl.vl.lock();
							if (!pl.view_list.contains(ID()))
							{
								pl.view_list.insert(ID());
								pl.vl.unlock();

								//put
								pl.send_put_packet(ID());
							}
							else
							{
								pl.vl.unlock();

								//move
								pl.send_move_packet(ID(), 0);
							}
						}
						else // ������
						{
							vl.unlock();

							//���� ������� ������
							pl.vl.lock();
							if (!pl.view_list.contains(ID()))
							{
								pl.view_list.insert(ID());
								pl.vl.unlock();

								//put
								pl.send_put_packet(ID());
							}
							else
							{
								pl.vl.unlock();

								//move
								pl.send_move_packet(ID(), 0);
							}
						}
					}
				}
			}
		}


		//send_stat_change_packet(data.sc_id);

	}
	else
	{
		//��Ȱ Ƽ�� �ɾ����
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