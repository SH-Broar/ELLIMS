#include "SESSION.h"
#include "DataBaseManager.h"

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
	SC_MOVE_PLAYER_PACKET p;
	p.id = c_id;
	p.size = sizeof(SC_MOVE_PLAYER_PACKET);
	p.type = SC_MOVE_PLAYER;
	p.x = clients[c_id].X();
	p.y = clients[c_id].Y();
	p.client_time = client_time;
	do_send(&p);
}

void SESSION::send_remove_packet(int c_id)
{
	SC_REMOVE_PLAYER_PACKET p;
	p.id = c_id;
	p.size = sizeof(p);
	p.type = SC_REMOVE_PLAYER;
	do_send(&p);
}

void SESSION::send_chat_packet(int c_id, char* mess)
{
	SC_CHAT_PACKET p;
	p.id = c_id;
	p.size = sizeof(SC_CHAT_PACKET) - sizeof(p.mess) + strlen(mess) + 1;
	p.type = SC_CHAT;

	//System Message
	if (c_id < 0)
	{
		strcpy_s(p.name, "_SYSTEM");
		strcpy_s(p.mess, mess);
		printf(" : %s", p.mess);
	}
	else
	{
		strcpy_s(p.name, clients[c_id].NAME());
		strcpy_s(p.mess, mess);
		printf("chat : %s\n", p.mess);
	}

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

void SESSION::	do_recv()
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
	SC_LOGIN_INFO_PACKET p;
	p.id = data.sc_id;
	strcpy(p.name, data.name);
	p.size = sizeof(SC_LOGIN_INFO_PACKET);
	p.type = SC_LOGIN_INFO;
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

void SESSION::send_put_packet(int c_id)
{
	SC_ADD_PLAYER_PACKET put_packet;
	put_packet.id = c_id;
	//strcpy(put_packet.name, data.name);
	strcpy_s(put_packet.name, clients[c_id].NAME());
	put_packet.size = sizeof(put_packet);
	put_packet.type = SC_ADD_PLAYER;
	put_packet.x = clients[c_id].X();
	put_packet.y = clients[c_id].Y();
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

	data.x = _x;
	data.y = _y;

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