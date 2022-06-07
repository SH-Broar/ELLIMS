#pragma once
#include "AMBIT.h"
#include "main.h"

enum SESSION_STATE;
enum COMP_TYPE;

class OVER_EXP
{
public:
	WSAOVERLAPPED _over;	//클래스 헤드 주소와 같은 위치의 주소를 가져오게 하기 위해 맨 처음에 씀
							//원래는 map 써야되는데 너무 복잡하고 오버헤드도 크고 send는 변경도 자주 있기때문에 그냥 이렇게 하셈
	WSABUF _wsabuf;
	char _send_buf[BUF_SIZE];
	COMP_TYPE _comp_type;

	OVER_EXP();

	OVER_EXP(unsigned char* packet);
};


class SESSION
{
	OVER_EXP _recv_over;
	short x, y;

public:
	std::mutex _sl;
	SESSION_STATE _s_state;
	int _id;
	SOCKET _socket;
	short sectorX, sectorY;	//한 사람이 동시에 두번 움직이진 않고, 남의 sector를 바꿀 일이 없으므로 atomic이 아님
	char _name[NAME_SIZE];
	int _prev_remain;

	std::unordered_set <int> view_list;
	std::mutex	vl;

public:

	SESSION();

	~SESSION();

	void do_recv();

	void do_send(void* packet);

	void send_login_info_packet();
	void send_put_packet(int c_id);
	void send_move_packet(int c_id, int client_time);
	void send_remove_packet(int c_id);
	void send_chat_packet(int c_id, char* mess);

	void setXY(short x, short y);

	short X();
	short Y();

	friend int distance(int a, int b);

};