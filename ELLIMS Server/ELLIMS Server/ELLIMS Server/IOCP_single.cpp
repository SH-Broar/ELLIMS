//***********************************
// 채팅서버
//***********************************

#include <iostream>
#include <array>
//#include <concurrent_unordered_map.h>
#include <WS2tcpip.h>
#include <MSWSock.h>

#pragma comment(lib,"WS2_32.lib")
#pragma comment(lib,"MSWSock.lib")

#include "protocol.h"
using namespace std;


constexpr int MAX_USER = 10;

class SESSION;



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
	bool in_use;
	int _id;
	SOCKET _socket;
	short x, y;
	char _name[NAME_SIZE];
	int _prev_remain;

public:

	SESSION()
	{
		_id = -1;
		_socket = 0;
		x = y = 0;
		_name[0] = 0;
		in_use = false;
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
		if (0 != ret)
		{
			int err_no = WSAGetLastError();
			if (err_no != WSA_IO_PENDING)
				error_display("WSARecv : ", err_no);
		}

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

	void send_move_packet(int c_id);
};

array<SESSION, MAX_USER> clients;

void SESSION::send_move_packet(int c_id)
{
	SC_MOVE_PLAYER_PACKET p;
	p.id = c_id;
	p.size = sizeof(SC_MOVE_PLAYER_PACKET);
	p.type = SC_MOVE_PLAYER;
	p.x = clients[c_id].x;
	p.y = clients[c_id].y;
	do_send(&p);
}

int get_new_client_id()
{
	for (int i = 0; i < MAX_USER; ++i)
	{
		if (clients[i].in_use == false)
		{
			return i;
		}
	}
	return -1;
}

void process_packet(int c_id, char* packet)
{
	switch (packet[1])
	{
	case CS_LOGIN:
	{
		CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(packet);
		strcpy_s(clients[c_id]._name, p->name);
		clients[c_id].send_login_info_packet();

		for (auto& pl : clients)
		{
			if (pl.in_use == false) continue;
			if (pl._id == c_id) continue;
			SC_ADD_PLAYER_PACKET add_packet;
			add_packet.id = c_id;
			strcpy_s(add_packet.name, p->name);
			add_packet.size = sizeof(add_packet);
			add_packet.type = SC_ADD_PLAYER;
			add_packet.x = clients[c_id].x;
			add_packet.y = clients[c_id].y;
			pl.do_send(&add_packet);
		}

		for (auto& pl : clients)
		{
			if (pl.in_use == false) continue;
			if (pl._id == c_id) continue;
			SC_ADD_PLAYER_PACKET add_packet;
			add_packet.id = pl._id;
			strcpy_s(add_packet.name, pl._name);
			add_packet.size = sizeof(add_packet);
			add_packet.type = SC_ADD_PLAYER;
			add_packet.x = pl.x;
			add_packet.y = pl.y;
			clients[c_id].do_send(&add_packet);
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

		for (auto& pl : clients)
		{
			if (true == pl.in_use)
				pl.send_move_packet(c_id);
		}


		break;
	}
	}
}

void disconnect(int c_id)
{
	for (auto& pl : clients)
	{
		if (pl.in_use == false) continue;
		if (pl._id == c_id) continue;
		SC_REMOVE_PLAYER_PACKET p;
		p.id = c_id;
		p.size = sizeof(p);
		p.type = SC_REMOVE_PLAYER;
		pl.do_send(&p);
	}

	closesocket(clients[c_id]._socket);
	clients[c_id].in_use = false;

}

int main()
{
	HANDLE h_iocp;

	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);

	SOCKET server = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN server_addr;

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT_NUM);
	server_addr.sin_addr.S_un.S_addr = INADDR_ANY;

	bind(server, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	listen(server, SOMAXCONN);

	SOCKADDR_IN cl_addr;
	int addr_size = sizeof(cl_addr);

	int client_id = 0;

	h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(server), h_iocp, 9999, 0);
	SOCKET c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	OVER_EXP accept_over;
	accept_over._comp_type = OP_ACCEPT;

	AcceptEx(server, c_socket, accept_over._send_buf, 0, addr_size + 16, addr_size + 16, 0, &accept_over._over);


	while (true)
	{
		DWORD num_bytes;
		ULONG_PTR key;
		WSAOVERLAPPED* over;
		BOOL ret = GetQueuedCompletionStatus(h_iocp, &num_bytes, &key, &over, INFINITE);
		OVER_EXP* ex_over = reinterpret_cast<OVER_EXP*>(over);

		if (FALSE == ret)
		{
			if (ex_over->_comp_type == OP_ACCEPT) cout << "Accept ERROR";
			else
			{
				cout << "GQCS error on client[" << key << "]\n";
				disconnect(key);
				if (ex_over->_comp_type == OP_SEND) delete ex_over;
				continue;
			}
		}

		switch (ex_over->_comp_type)
		{
		case OP_ACCEPT:
		{
			int client_id = get_new_client_id();
			if (client_id != -1)
			{
				clients[client_id].in_use = true;
				clients[client_id].x = 0;
				clients[client_id].y = 0;
				clients[client_id]._id = client_id;
				clients[client_id]._name[0] = 0;
				clients[client_id]._socket = c_socket;
				clients[client_id]._prev_remain = 0;
				CreateIoCompletionPort(reinterpret_cast<HANDLE>(c_socket), h_iocp, client_id, 0);

				clients[client_id].do_recv();
				c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
			}
			else
			{
				cout << "Max user exceeded.\n";
			}
			ZeroMemory(&accept_over._over, sizeof(accept_over._over));
			AcceptEx(server, c_socket, accept_over._send_buf, 0, addr_size + 16, addr_size + 16, 0, &accept_over._over);
			break;
		}
		case OP_RECV:
		{
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
			delete ex_over;
			break;
		}

	}

	closesocket(server);
	WSACleanup();
}

