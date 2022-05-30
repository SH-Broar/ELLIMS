//***********************************
// 채팅서버
//***********************************

#include <iostream>
#include <array>
//#include <concurrent_unordered_map.h>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <vector>
#include <thread>
#include <mutex>
#include <unordered_set>

#pragma comment(lib,"WS2_32.lib")
#pragma comment(lib,"MSWSock.lib")

#include "protocol.h"
using namespace std;


class SESSION;
constexpr int RANGE = 5;


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

enum COMP_TYPE { OP_ACCEPT, OP_RECV, OP_SEND };

enum SESSION_STATE { ST_FREE, ST_ACCEPTED, ST_INGAME };

class OVER_EXP
{
public:
	WSAOVERLAPPED _over;	//클래스 헤드 주소와 같은 위치의 주소를 가져오게 하기 위해 맨 처음에 씀
							//원래는 map 써야되는데 너무 복잡하고 오버헤드도 크고 send는 변경도 자주 있기때문에 그냥 이렇게 하셈
	WSABUF _wsabuf;
	char _send_buf[BUF_SIZE];
	COMP_TYPE _comp_type;

	OVER_EXP()
	{
		_wsabuf.len = BUF_SIZE;
		_wsabuf.buf = _send_buf;
		_comp_type = OP_RECV;
		ZeroMemory(&_over, sizeof(_over));
	};

	OVER_EXP(char* packet)
	{
		_wsabuf.len = packet[0];
		_wsabuf.buf = _send_buf;
		ZeroMemory(&_over, sizeof(_over));
		_comp_type = OP_SEND;
		memcpy(_send_buf, packet, packet[0]);
	};
};

class SESSION
{
	OVER_EXP _recv_over;

public:
	mutex _sl;
	SESSION_STATE _s_state;
	int _id;
	SOCKET _socket;
	short x, y;
	char _name[NAME_SIZE];
	int _prev_remain;

	unordered_set <int> view_list;
	mutex	vl;

public:

	SESSION()
	{
		_id = -1;
		_socket = 0;
		x = y = 0;
		_name[0] = 0;
		_s_state = ST_FREE;
		_prev_remain = 0;
	}

	~SESSION()
	{

	}

	void do_recv()
	{
		DWORD recv_flag = 0;
		memset(&_recv_over._over, 0, sizeof(_recv_over._over));
		_recv_over._wsabuf.len = BUF_SIZE - _prev_remain;
		_recv_over._wsabuf.buf = _recv_over._send_buf + _prev_remain;

		int ret = WSARecv(_socket, &_recv_over._wsabuf, 1, 0, &recv_flag, &_recv_over._over, 0);
		//if (0 != ret)
		//{
		//	int err_no = WSAGetLastError();
		//	if (err_no != WSA_IO_PENDING)
		//		error_display("WSARecv : ", err_no);
		//}

	}

	void do_send(void* packet)
	{

		OVER_EXP* sdata = new OVER_EXP{ reinterpret_cast<char*>(packet) };

		WSASend(_socket, &sdata->_wsabuf, 1, 0, 0, &sdata->_over, 0);
	}

	void send_login_info_packet()
	{
		SC_LOGIN_INFO_PACKET p;
		p.id = _id;
		p.size = sizeof(SC_LOGIN_INFO_PACKET);
		p.type = SC_LOGIN_INFO;
		p.x = x;
		p.y = y;
		do_send(&p);
	}

	void send_put_packet(int c_id);
	void send_move_packet(int c_id, int client_time);

	void send_remove_packet(int c_id)
	{
		SC_REMOVE_PLAYER_PACKET p;
		p.id = c_id;
		p.size = sizeof(p);
		p.type = SC_REMOVE_PLAYER;
		do_send(&p);
	}
};

array<SESSION, MAX_USER> clients;
HANDLE g_h_iocp;
SOCKET g_s_socket;

int distance(int a, int b)
{
	return abs(clients[a].x - clients[b].x) + abs(clients[a].y - clients[b].y);
}

void SESSION::send_move_packet(int c_id, int client_time)
{
	SC_MOVE_PLAYER_PACKET p;
	p.id = c_id;
	p.size = sizeof(SC_MOVE_PLAYER_PACKET);
	p.type = SC_MOVE_PLAYER;
	p.x = clients[c_id].x;
	p.y = clients[c_id].y;
	p.client_time = client_time;
	do_send(&p);
}

void SESSION::	send_put_packet(int c_id)
{
	SC_ADD_PLAYER_PACKET put_packet;
	put_packet.id = c_id;
	strcpy_s(put_packet.name, clients[c_id]._name);
	put_packet.size = sizeof(put_packet);
	put_packet.type = SC_ADD_PLAYER;
	put_packet.x = clients[c_id].x;
	put_packet.y = clients[c_id].y;
	do_send(&put_packet);
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
	clients[c_id]._sl.unlock();

	for (auto& pl : clients)
	{
		if (pl._id == c_id) continue;
		pl._sl.lock();
		if (pl._s_state != ST_INGAME)
		{
			pl._sl.unlock();
			continue;
		} //읽기만 하는데요?
		//쓰기 작업의 원인이 되는 조건이잖아

		pl.send_remove_packet(c_id);
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
			clients[c_id]._sl.unlock();
			disconnect(c_id);
			break;
		}

		strcpy_s(clients[c_id]._name, p->name);
		clients[c_id].send_login_info_packet();
		clients[c_id]._s_state = ST_INGAME;
		clients[c_id]._sl.unlock();

		clients[c_id].x = rand() % W_WIDTH;
		clients[c_id].y = rand() % W_HEIGHT;
		//clients[c_id].x = 0;
		//clients[c_id].y = 0;

		//-> pl
		for (auto& pl : clients)
		{
			if (pl._id == c_id) continue;

			pl._sl.lock();
			if (pl._s_state != ST_INGAME)
			{
				pl._sl.unlock();
				continue;
			}

			if (RANGE >= distance(c_id, pl._id))
			{
				pl.vl.lock();
				pl.view_list.insert(c_id);
				pl.vl.unlock();

				pl.send_put_packet(c_id);
			}
			pl._sl.unlock();
		}

		//-> c_id
		for (auto& pl : clients)
		{
			if (pl._id == c_id) continue;
			lock_guard<mutex> aa{ pl._sl };	//편한 언락
			if (pl._s_state != ST_INGAME)continue;

			if (RANGE >= distance(c_id, pl._id))
			{
				clients[c_id].vl.lock();
				clients[c_id].view_list.insert(pl._id);
				clients[c_id].vl.unlock();

				clients[c_id].send_put_packet(pl._id);
			}
		}

		break;
	}
	case CS_MOVE:
	{
		CS_MOVE_PACKET* p = reinterpret_cast<CS_MOVE_PACKET*>(packet);

		short x = clients[c_id].x;
		short y = clients[c_id].y;
		switch (p->direction)
		{
		case 0: if (y > 0)y--; break;
		case 1: if (y < W_HEIGHT - 1)y++; break;
		case 2: if (x > 0)x--; break;
		case 3: if (x < W_WIDTH - 1)x++; break;
		}
		clients[c_id].x = x;
		clients[c_id].y = y;

		//clients[c_id].vl.lock();
		//unordered_set<int> p_vlRapper = clients[c_id].view_list;
		//clients[c_id].vl.unlock();

		//-> 모든 클라이언트에게 (자신 포함)
		for (auto& pl : clients)
		{
			if (pl._id == c_id)
			{
				pl.send_move_packet(c_id, p->client_time);
				continue;
			}
			lock_guard<mutex> aa{ pl._sl };	//편한 언락
			if (pl._s_state != ST_INGAME)continue;

			//near의 모든 객체에 대해?
			if (distance(c_id, pl._id) <= RANGE)
			{

				clients[c_id].vl.lock();
				//없으면
				if (clients[c_id].view_list.find(pl._id) == clients[c_id].view_list.end())
				{
					clients[c_id].view_list.insert(pl._id);
					clients[c_id].vl.unlock();

					//put
					clients[c_id].send_put_packet(pl._id);

					//pl.vl.lock();
					//unordered_set<int> o_vlRapper = pl.view_list;
					//pl.vl.unlock();

					//내가 상대한테 없으면
					pl.vl.lock();
					if (pl.view_list.find(clients[c_id]._id) == pl.view_list.end())
					{
						pl.view_list.insert(clients[c_id]._id);
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
					//pl.vl.lock
					// 
					//unordered_set<int> o_vlRapper = pl.view_list;
					//pl.vl.unlock();

					//내가 상대한테 없으면
					pl.vl.lock();
					if (pl.view_list.find(clients[c_id]._id) == pl.view_list.end())
					{
						pl.view_list.insert(clients[c_id]._id);
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

		//-----------

		clients[c_id].vl.lock();
		unordered_set<int> r_view_list = clients[c_id].view_list;
		clients[c_id].vl.unlock();

		for (auto ids : r_view_list)
		{
			if (distance(ids, c_id) > RANGE)
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
				clients[client_id].x = 0;
				clients[client_id].y = 0;
				clients[client_id]._id = client_id;
				clients[client_id]._name[0] = 0;
				clients[client_id]._socket = c_socket;
				clients[client_id]._prev_remain = 0;
				CreateIoCompletionPort(reinterpret_cast<HANDLE>(c_socket), g_h_iocp, client_id, 0);

				clients[client_id].do_recv();
				c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
			}
			else
			{
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
		}

	}
}

int main()
{
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


	vector <thread> worker_threads;

	//이거 1로 하면 싱글 스레드로
	for (int i = 0; i < 6; i++)
	{
		worker_threads.emplace_back(do_worker);
	}
	for (auto& th : worker_threads)
	{
		th.join();
	}



	closesocket(g_s_socket);
	WSACleanup();
}

