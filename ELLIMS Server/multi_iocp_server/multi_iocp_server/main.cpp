#include "AMBIT.h"
#include "main.h"

array<SESSION, MAX_USER + NUM_NPC> clients;
HANDLE g_h_iocp;
SOCKET g_s_socket;

concurrent_unordered_map<int, concurrent_unordered_map<int, concurrent_unordered_map<int, bool>>> sectors;

void error_display(const char* msg, int err_no)
{
	WCHAR* lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	std::cout << msg;
	std::wcout << L"에러 " << lpMsgBuf << std::endl;
	while (true);
	LocalFree(lpMsgBuf);
}

int distance_cell(int a, int b)
{
	return abs(clients[a].X() - clients[b].X()) + abs(clients[a].Y() - clients[b].Y());
}

int get_new_client_id()
{
	for (int i = 0; i < MAX_USER; ++i)
	{
		clients[i]._sl.lock();
		if (clients[i]._s_state == ST_FREE)
		{
			clients[i]._s_state = ST_ACCEPTED;
			clients[i]._sl.unlock();
			return i;
		}
		clients[i]._sl.unlock();
	}
	return -1;
}

void disconnect(int c_id)
{
	clients[c_id]._sl.lock();
	if (clients[c_id]._s_state == ST_FREE)
	{
		clients[c_id]._sl.unlock();
		return;
	}
	closesocket(clients[c_id]._socket);
	clients[c_id]._s_state = ST_FREE;
	DataBaseManager::addDBEvent(c_id, DB_EV_LOGOUT, clients[c_id].getData());
	clients[c_id]._sl.unlock();

	for (auto& pl : clients)
	{
		if (pl.ID() == c_id) continue;
		if (pl.ID() >= MAX_USER) continue;
		pl._sl.lock();
		if (pl._s_state != ST_INGAME)
		{
			pl._sl.unlock();
			continue;
		} //읽기만 하는데요?
		//쓰기 작업의 원인이 되는 조건이잖아

		pl.vl.lock();
		if (0 != pl.view_list.count(c_id))
		{
			pl.view_list.erase(c_id);
			pl.vl.unlock();
			pl.send_remove_packet(c_id);
		}
		else
			pl.vl.unlock();
		pl._sl.unlock();
	}

}

void process_packet(int c_id, char* packet)
{
	switch (packet[1])
	{
	case CS_LOGIN:
	{
		CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(packet);

		clients[c_id]._sl.lock();
		if (clients[c_id]._s_state == ST_FREE)
		{
			clients[c_id]._sl.unlock();
			break;
		}
		if (clients[c_id]._s_state == ST_INGAME)
		{
			clients[c_id].send_login_fail_packet(1);
			clients[c_id]._sl.unlock();
			disconnect(c_id);
			break;
		}

		//DB 스레드
		LoginData dbTMP;
		strcpy(dbTMP.name, p->name);
		strcpy(dbTMP.pass, p->pass);
		DataBaseManager::addDBEvent(c_id, DB_EV_LOGIN, dbTMP);

		clients[c_id]._sl.unlock();

		break;
	}
	case CS_ATTACK:
	{
		CS_ATTACK_PACKET* p = reinterpret_cast<CS_ATTACK_PACKET*>(packet);
		//
		std::vector<COORD> range;
		range.reserve(10);
		switch (p->skilltype)
		{
		case 0:
			range.emplace_back(1, 0);
			range.emplace_back(0, 1);
			range.emplace_back(0, -1);
			range.emplace_back(-1, 0);
			break;
		}

		clients[c_id].vl.lock();
		unordered_set<int> r_view_list = clients[c_id].view_list;
		clients[c_id].vl.unlock();

		for (auto& pln : r_view_list)
		{
			//cout << "\nGET";
			if (clients[c_id].getData().isPlayer == clients[pln].getData().isPlayer)
				continue;

			int hitted = 0;

			auto& pl = clients[pln];

			for (auto cd : range)
			{
				if (pl.X() == clients[c_id].X() + cd.X && pl.Y() == clients[c_id].Y() + cd.Y)
				{
					if (pl._s_state != ST_NPC_DEAD)
					{
						//피격
						//cout << "\nHITTED";
						hitted = rand() % 5 + clients[c_id].getData().level;
						pl._sl.lock();
						pl.setDamage(hitted);
						pl._sl.unlock();
						break;
					}
				}
			}

			if (hitted > 0)
			{
				string mess;
				mess = clients[c_id].getData().name;
				mess += " attack ";
				mess += pl.getData().name;
				mess += " by ";
				mess += to_string(hitted);
				clients[c_id].send_chat_packet(-1, mess.c_str());
				clients[c_id].send_stat_change_packet(pln);
				for (auto& every : r_view_list)
				{
					if (!clients[every].getData().isPlayer)
						continue;

					clients[every].send_stat_change_packet(pln);
				}
			}
			else if (hitted < 0)
			{
				clients[c_id]._sl.lock();
				bool levelup = clients[c_id].appendEXP((pl.getData().level + 3) * pl.getData().level * 2);
				clients[c_id]._sl.unlock();

				string mess;
				mess = pl.getData().name;
				mess += " collapsed!";
				clients[c_id].send_chat_packet(-1, mess.c_str());
				clients[c_id].send_stat_change_packet(c_id);
				clients[c_id].send_remove_packet(pln);
				for (auto& every : r_view_list)
				{
					if (!clients[every].getData().isPlayer)
						continue;

					clients[every].send_stat_change_packet(c_id);
					clients[every].send_remove_packet(pln);

					if (levelup)
					{
						string lmess;
						lmess = clients[c_id].getData().name;
						lmess += " level up to ";
						lmess += clients[c_id].getData().level;
						clients[every].send_chat_packet(-1, lmess.c_str());
					}
				}
			}
		}

		break;
	}
	case CS_MOVE:
	{
		CS_MOVE_PACKET* p = reinterpret_cast<CS_MOVE_PACKET*>(packet);

		short x = clients[c_id].X();
		short y = clients[c_id].Y();

		switch (p->direction)
		{
		case -1:
			x = W_WIDTH / 2;
			y = W_HEIGHT / 2;
			break;
		case 0:
			if (y > 0 && s_Map::canMove(x, y - 1))y--;
			break;
		case 1:
			if (y < W_HEIGHT - 1 && s_Map::canMove(x, y + 1))y++;
			break;
		case 2:
			if (x > 0 && s_Map::canMove(x - 1, y))x--;
			break;
		case 3:
			if (x < W_WIDTH - 1 && s_Map::canMove(x + 1, y))x++;
			break;
		}

		clients[c_id].setXY(x, y);


		//-> 섹터 내의 모든 클라이언트에게 (자신 포함)
		for (int i = 0; i < 3; ++i)
		{
			for (int h = 0; h < 3; ++h)
			{
				if (clients[c_id].sectorX - 1 + i < 0 ||
					clients[c_id].sectorX - 1 + i >= W_WIDTH / SECTOR_WIDTH ||
					clients[c_id].sectorY - 1 + h < 0 ||
					clients[c_id].sectorY - 1 + h >= W_HEIGHT / SECTOR_HEIGHT)
					continue;

				for (auto& pln : sectors[clients[c_id].sectorX - 1 + i][clients[c_id].sectorY - 1 + h])
				{
					if (pln.second == false)
						continue;
					if (pln.first == c_id)
					{
						clients[c_id].send_move_packet(c_id, p->client_time);
						continue;
					}

					auto& pl = clients[pln.first];

					//범위 내의 NPC 깨우기
					if (pl._s_state == ST_NPC_SLEEP)
					{
						SESSION_STATE sst = ST_NPC_SLEEP;
						if (atomic_compare_exchange_weak(&(pl._s_state), &sst, ST_INGAME))
						{
							HeartManager::add_timer(pl.npc_id, 1000, EV_MOVE, c_id);
						}
					}

					if (pl.ID() >= MAX_USER) continue;
					lock_guard<mutex> aa{ pl._sl };	//편한 언락

					if (pl._s_state != ST_INGAME)continue;

					//near의 모든 객체에 대해?
					if (distance_cell(c_id, pl.ID()) <= RANGE)
					{
						clients[c_id].vl.lock();
						//없으면
						if (!clients[c_id].view_list.contains(pl.ID()))
						{
							clients[c_id].view_list.insert(pl.ID());
							clients[c_id].vl.unlock();

							//put
							clients[c_id].send_put_packet(pl.ID());


							//내가 상대한테 없으면
							pl.vl.lock();
							if (!pl.view_list.contains(clients[c_id].ID()))
							{
								pl.view_list.insert(clients[c_id].ID());
								pl.vl.unlock();

								//put
								pl.send_put_packet(c_id);
							}
							else
							{
								pl.vl.unlock();

								//move
								pl.send_move_packet(c_id, p->client_time);
							}
						}
						else // 있으면
						{
							clients[c_id].vl.unlock();

							//내가 상대한테 없으면
							pl.vl.lock();
							if (!pl.view_list.contains(clients[c_id].ID()))
							{
								pl.view_list.insert(clients[c_id].ID());
								pl.vl.unlock();

								//put
								pl.send_put_packet(c_id);
							}
							else
							{
								pl.vl.unlock();

								//move
								pl.send_move_packet(c_id, p->client_time);
							}
						}
					}
				}
			}
		}

		//-----------

		clients[c_id].vl.lock();
		unordered_set<int> r_view_list = clients[c_id].view_list;
		clients[c_id].vl.unlock();

		for (auto ids : r_view_list)
		{
			if (distance_cell(ids, c_id) > RANGE)
			{
				clients[c_id].vl.lock();
				clients[c_id].view_list.erase(ids);
				clients[c_id].vl.unlock();

				//remove
				clients[c_id].send_remove_packet(ids);

				clients[ids].vl.lock();
				if (clients[ids].view_list.find(c_id) != clients[ids].view_list.end())
				{
					clients[ids].view_list.erase(c_id);
					clients[ids].vl.unlock();

					//remove
					clients[ids].send_remove_packet(c_id);
				}
				else
				{
					clients[ids].vl.unlock();
				}
			}
		}

		break;
	}
	case CS_CHAT:
	{
		CS_CHAT_PACKET* p = reinterpret_cast<CS_CHAT_PACKET*>(packet);


		for (int i = 0; i < 3; ++i)
		{
			for (int h = 0; h < 3; ++h)
			{
				if (clients[c_id].sectorX - 1 + i < 0 ||
					clients[c_id].sectorX - 1 + i >= W_WIDTH / SECTOR_WIDTH ||
					clients[c_id].sectorY - 1 + h < 0 ||
					clients[c_id].sectorY - 1 + h >= W_HEIGHT / SECTOR_HEIGHT)
					continue;

				for (auto& pln : sectors[clients[c_id].sectorX - 1 + i][clients[c_id].sectorY - 1 + h])
				{
					if (pln.second)
					{
						if (pln.first < MAX_USER)
							clients[pln.first].send_chat_packet(c_id, p->mess);
					}
				}
			}
		}

		break;
	}
	}

}

void do_worker()
{

	while (true)
	{
		DWORD num_bytes;
		ULONG_PTR key;
		WSAOVERLAPPED* over;
		BOOL ret = GetQueuedCompletionStatus(g_h_iocp, &num_bytes, &key, &over, INFINITE);
		OVER_EXP* ex_over = reinterpret_cast<OVER_EXP*>(over);

		if (FALSE == ret)
		{
			if (ex_over->_comp_type == OP_ACCEPT) cout << "Accept ERROR";
			else
			{
				cout << "GQCS error on client[" << key << "]\n";
				disconnect(static_cast<int>(key));
				if (ex_over->_comp_type == OP_SEND) delete ex_over;
				continue;
			}
		}

		switch (ex_over->_comp_type)
		{
		case OP_ACCEPT:
		{
			SOCKET c_socket = reinterpret_cast<SOCKET>(ex_over->_wsabuf.buf);
			int client_id = get_new_client_id();
			if (client_id != -1)
			{

				printf("Login : %d\n", client_id);
				clients[client_id].ID(1);
				clients[client_id].setXY(0, 0);
				sprintf(clients[client_id].NAME(), "tmp");
				clients[client_id]._socket = c_socket;
				clients[client_id]._prev_remain = 0;
				CreateIoCompletionPort(reinterpret_cast<HANDLE>(c_socket), g_h_iocp, client_id, 0);

				clients[client_id].do_recv();
				c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
			}
			else
			{
				clients[client_id].send_login_fail_packet(2);
				cout << "Max user exceeded.\n";
			}
			ZeroMemory(&ex_over->_over, sizeof(ex_over->_over));
			ex_over->_wsabuf.buf = reinterpret_cast<CHAR*>(c_socket);
			int addr_size = sizeof(SOCKADDR_IN);
			AcceptEx(g_s_socket, c_socket, ex_over->_send_buf, 0, addr_size + 16, addr_size + 16, 0, &ex_over->_over);
			break;
		}
		case OP_RECV:
		{
			if (0 == num_bytes)
			{
				disconnect(key);
			}
			int remain_data = num_bytes + clients[key]._prev_remain;
			char* p = ex_over->_send_buf;

			while (remain_data > 0)
			{
				int packet_size = p[0];
				if (packet_size <= remain_data)
				{
					process_packet(static_cast<int>(key), p);
					p = p + packet_size;
					remain_data = remain_data - packet_size;
				}
				else
				{
					break;
				}
			}
			clients[key]._prev_remain = remain_data;
			if (remain_data > 0)
			{
				memcpy(ex_over->_send_buf, p, remain_data);
			}

			clients[key].do_recv();
			break;
		}
		case OP_SEND:
			if (0 == num_bytes)
			{
				disconnect(key);
			}
			delete ex_over;
			break;

		case OP_DB:
		{
			OVER_DB* db_over = reinterpret_cast<OVER_DB*>(ex_over);
			switch (db_over->_db_type)
			{
			case DB_EV_LOGIN:
			{
				LoginData Data = db_over->datas;
				int c_id = key;
				Data.sc_id = c_id;

				clients[c_id]._sl.lock();

				clients[c_id].setData(Data);
				clients[c_id].send_login_info_packet();
				if (!Data.isValidLogin)
				{
					cout << "This is Not ValidLogin";
					// 아이디 생성
					clients[c_id]._sl.unlock();
					break;
				}
				else
				{
					cout << "Login Success : " << Data.name;
					clients[c_id]._s_state = ST_INGAME;
					clients[c_id]._sl.unlock();

					//-> 섹터 내의 모든 클라이언트에게 (자신 포함)
					for (int i = 0; i < 3; ++i)
					{
						for (int h = 0; h < 3; ++h)
						{
							if (clients[c_id].sectorX - 1 + i < 0 ||
								clients[c_id].sectorX - 1 + i >= W_WIDTH / SECTOR_WIDTH ||
								clients[c_id].sectorY - 1 + h < 0 ||
								clients[c_id].sectorY - 1 + h >= W_HEIGHT / SECTOR_HEIGHT)
								continue;

							for (auto& pln : sectors[clients[c_id].sectorX - 1 + i][clients[c_id].sectorY - 1 + h])
							{
								if (pln.second == false)
									continue;
								if (pln.first == c_id)
								{
									clients[c_id].send_move_packet(c_id, 0);
									continue;
								}

								auto& pl = clients[pln.first];

								//범위 내의 NPC 깨우기
								if (pl._s_state == ST_NPC_SLEEP)
								{
									SESSION_STATE sst = ST_NPC_SLEEP;
									if (atomic_compare_exchange_weak(&(pl._s_state), &sst, ST_INGAME))
									{
										HeartManager::add_timer(pl.npc_id, 1000, EV_MOVE, c_id);
									}
								}

								if (pl.ID() >= MAX_USER) continue;
								lock_guard<mutex> aa{ pl._sl };	//편한 언락

								if (pl._s_state != ST_INGAME)continue;

								//near의 모든 객체에 대해?
								if (distance_cell(c_id, pl.ID()) <= RANGE)
								{
									clients[c_id].vl.lock();
									//없으면
									if (!clients[c_id].view_list.contains(pl.ID()))
									{
										clients[c_id].view_list.insert(pl.ID());
										clients[c_id].vl.unlock();

										//put
										clients[c_id].send_put_packet(pl.ID());


										//내가 상대한테 없으면
										pl.vl.lock();
										if (!pl.view_list.contains(clients[c_id].ID()))
										{
											pl.view_list.insert(clients[c_id].ID());
											pl.vl.unlock();

											//put
											pl.send_put_packet(c_id);
										}
										else
										{
											pl.vl.unlock();

											//move
											pl.send_move_packet(c_id, 0);
										}
									}
									else // 있으면
									{
										clients[c_id].vl.unlock();

										//내가 상대한테 없으면
										pl.vl.lock();
										if (!pl.view_list.contains(clients[c_id].ID()))
										{
											pl.view_list.insert(clients[c_id].ID());
											pl.vl.unlock();

											//put
											pl.send_put_packet(c_id);
										}
										else
										{
											pl.vl.unlock();

											//move
											pl.send_move_packet(c_id, 0);
										}
									}
								}
							}
						}
					}

					////view list

					//for (auto& pl : clients)
					//{
					//	if (pl.ID() == c_id) continue;

					//	pl._sl.lock();
					//	if (pl._s_state != ST_INGAME)
					//	{
					//		pl._sl.unlock();
					//		continue;
					//	}

					//	if (RANGE >= distance_cell(c_id, pl.ID()))
					//	{
					//		pl.vl.lock();
					//		pl.view_list.insert(c_id);
					//		pl.vl.unlock();

					//		pl.send_put_packet(c_id);
					//	}
					//	pl._sl.unlock();
					//}

					////-> c_id
					//for (auto& pl : clients)
					//{
					//	if (pl.ID() == c_id) continue;
					//	lock_guard<mutex> aa{ pl._sl };	//편한 언락
					//	if (pl._s_state != ST_INGAME)continue;

					//	if (RANGE >= distance_cell(c_id, pl.ID()))
					//	{
					//		clients[c_id].vl.lock();
					//		clients[c_id].view_list.insert(pl.ID());
					//		clients[c_id].vl.unlock();

					//		clients[c_id].send_put_packet(pl.ID());
					//	}
					//}

				}

				HeartManager::add_timer(c_id, 5000, EV_HEAL, c_id);
				break;
			}
			}
			break;
		}
		case OP_AI:
		{
			OVER_AI* ai_over = reinterpret_cast<OVER_AI*>(ex_over);

			switch (ai_over->_timer_type)
			{
			case TIMER_EVENT_TYPE::EV_ATTACK:
			{
				int npc_id = ai_over->object_id;
				//처리
				std::vector<COORD> range;
				range.reserve(5);
				range.emplace_back(1, 0);
				range.emplace_back(0, 1);
				range.emplace_back(0, -1);
				range.emplace_back(-1, 0);

				for (int i = 0; i < 3; ++i)
				{
					for (int h = 0; h < 3; ++h)
					{
						if (clients[npc_id].sectorX - 1 + i < 0 ||
							clients[npc_id].sectorX - 1 + i >= W_WIDTH / SECTOR_WIDTH ||
							clients[npc_id].sectorY - 1 + h < 0 ||
							clients[npc_id].sectorY - 1 + h >= W_HEIGHT / SECTOR_HEIGHT)
							continue;

						for (auto& pln : sectors[clients[npc_id].sectorX - 1 + i][clients[npc_id].sectorY - 1 + h])
						{
							if (pln.second == false)
								continue;
							if (pln.first == npc_id)
								continue;

							auto& pl = clients[pln.first];
							if (pl.ID() >= MAX_USER) continue;

							int hitted = 0;

							pl._sl.lock();
							//cout << "Attemp ";
							if (abs(pl.getData().x - clients[npc_id].getData().x) +
								abs(pl.getData().y - clients[npc_id].getData().y) <= 1)
							{
								//피격
								hitted = clients[npc_id].getData().level + 1 + (rand() % 3);

								pl.setDamage(hitted);
								pl._sl.unlock();
							}
							else
							{
								pl._sl.unlock();
							}

							if (hitted > 0)
							{
								//cout << "Att" << endl;
								string mess;
								mess = clients[npc_id].getData().name;
								mess += " attack ";
								mess += pl.getData().name;
								mess += " by ";
								mess += to_string(hitted);
								pl.send_chat_packet(-1, mess.c_str());

								pl.vl.lock();
								unordered_set<int> r_view_list = pl.view_list;
								pl.vl.unlock();
								pl.send_stat_change_packet(pln.first);
								for (auto& every : r_view_list)
								{
									if (!clients[every].getData().isPlayer)
										continue;

									clients[every].send_stat_change_packet(pln.first);
								}
							}
							else if (hitted < 0)
							{
								string mess;
								mess = pl.getData().name;
								mess += " collapsed!";

								pl.vl.lock();
								unordered_set<int> r_view_list = pl.view_list;
								pl.vl.unlock();

								pl.send_chat_packet(-1, mess.c_str());
								pl.send_stat_change_packet(pln.first);
								for (auto& every : r_view_list)
								{
									if (!clients[every].getData().isPlayer)
										continue;

									clients[every].send_chat_packet(-1, mess.c_str());
									clients[every].send_stat_change_packet(pln.first);
									if (every != pln.first)
										clients[every].send_remove_packet(pln.first);
								}
							}
						}
					}
				}

				break;
			}
			case TIMER_EVENT_TYPE::EV_MOVE:
				HeartManager::move_npc(ai_over->object_id, ai_over->target_id);
				break;
			case TIMER_EVENT_TYPE::EV_HEAL:
				clients[ai_over->object_id]._sl.lock();
				clients[ai_over->object_id].autoHeal();
				clients[ai_over->object_id]._sl.unlock();
				for (auto& every : sectors[clients[ai_over->object_id].sectorX][clients[ai_over->object_id].sectorY])
				{
					if (every.second == false)
						continue;

					//cout << "\nSENDED";
					clients[every.first].send_stat_change_packet(ai_over->object_id);
				}
				HeartManager::add_timer(ai_over->object_id, 5000, EV_HEAL, ai_over->object_id);
				break;
			}
		}
		break;
		}

	}
}

int main()
{
	HeartManager::initialize_npc();

	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);

	g_s_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN server_addr;

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT_NUM);
	server_addr.sin_addr.S_un.S_addr = INADDR_ANY;

	bind(g_s_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	listen(g_s_socket, SOMAXCONN);

	SOCKADDR_IN cl_addr;
	int addr_size = sizeof(cl_addr);

	int client_id = 0;

	g_h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_s_socket), g_h_iocp, 9999, 0);
	SOCKET c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	OVER_EXP accept_over;
	accept_over._comp_type = OP_ACCEPT;
	accept_over._wsabuf.buf = reinterpret_cast<CHAR*>(c_socket);
	AcceptEx(g_s_socket, c_socket, accept_over._send_buf, 0, addr_size + 16, addr_size + 16, 0, &accept_over._over);

	s_Map::loadMap();

	vector <thread> worker_threads;

	//이거 1로 하면 싱글 스레드로
	for (int i = 0; i < 4; i++)
	{
		worker_threads.emplace_back(do_worker);
	}
	thread db_thread{ DataBaseManager::DBThread };
	thread ai_thread{ HeartManager::ai_thread };

	db_thread.join();
	for (auto& th : worker_threads)
	{
		th.join();
	}



	closesocket(g_s_socket);
	WSACleanup();
}

