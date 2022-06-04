#pragma once
#include "Turboc.h"


enum COMP_TYPE { OP_ACCEPT, OP_RECV, OP_SEND, OP_PLAYER_MOVE };

class OverlappedExtra {
public:
	WSAOVERLAPPED _over;
	WSABUF _wsabuf;
	char _send_buf[BUF_SIZE];
	COMP_TYPE _comp_type;
	int target_id;
	OverlappedExtra()
	{
		_wsabuf.len = BUF_SIZE;
		_wsabuf.buf = _send_buf;
		_comp_type = OP_RECV;
		ZeroMemory(&_over, sizeof(_over));
	}
	OverlappedExtra(char* packet)
	{
		_wsabuf.len = packet[0];
		_wsabuf.buf = _send_buf;
		ZeroMemory(&_over, sizeof(_over));
		_comp_type = OP_SEND;
		memcpy(_send_buf, packet, packet[0]);
	}
};

class Network
{
private:
	static void error_display(const char* msg, int err_no);

public:
	Network();
	~Network();

	static void NetworkCodex();
	static void SendPacket(void* packet);


private:
	static HANDLE g_h_iocp;
	static SOCKET g_c_socket;
	static OverlappedExtra recv_over;

	unsigned char packet_buf[BUF_SIZE];
	int prev_packet_data;
	int curr_packet_size;

};
