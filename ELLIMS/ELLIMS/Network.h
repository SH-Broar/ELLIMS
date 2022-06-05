#pragma once
#include "Turboc.h"

class Game;
class Player;

enum COMP_TYPE { OP_RECV, OP_SEND, OP_PLAYER_MOVE };

class OverlappedExtra {
public:
	WSAOVERLAPPED _over;
	WSABUF _wsabuf;
	char _overlapped_buf[BUF_SIZE];
	COMP_TYPE _comp_type;
	int target_id;
	OverlappedExtra()
	{
		_wsabuf.len = BUF_SIZE;
		_wsabuf.buf = _overlapped_buf;
		_comp_type = OP_RECV;
		ZeroMemory(&_over, sizeof(_over));
	}
	OverlappedExtra(char* packet)
	{
		_wsabuf.len = packet[0];
		_wsabuf.buf = _overlapped_buf;
		ZeroMemory(&_over, sizeof(_over));
		_comp_type = OP_SEND;
		memcpy(_overlapped_buf, packet, packet[0]);
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
	static void PacketProcess();

private:
	static void SendPacket(void* packet);
	static void RecvPacketProcess(unsigned char packet[]);

public:


private:
	static HANDLE g_h_iocp;
	static SOCKET g_c_socket;
	static OverlappedExtra recv_over;

	static unsigned char packet_buf[BUF_SIZE];
	static int prev_packet_data;
	static int curr_packet_size;

};