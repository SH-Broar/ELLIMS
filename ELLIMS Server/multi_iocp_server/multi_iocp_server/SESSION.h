#pragma once
#include "AMBIT.h"
#include "main.h"
#include "LoginData.h"

enum SESSION_STATE;
enum COMP_TYPE;


class OVER_EXP
{
public:
	WSAOVERLAPPED _over;	//클래스 헤드 주소와 같은 위치의 주소를 가져오게 하기 위해 맨 처음에 씀
	WSABUF _wsabuf;
	char _send_buf[BUF_SIZE];
	COMP_TYPE _comp_type;

	OVER_EXP();

	OVER_EXP(unsigned char* packet);
};

class OVER_DB : public OVER_EXP
{
public:
	LoginData datas;
	DB_EVENT_TYPE _db_type;
};

class OVER_AI : public OVER_EXP
{
public:
	int object_id;
	int target_id;
	TIMER_EVENT_TYPE _timer_type;
};

class SESSION
{
	LoginData data;
	OVER_EXP _recv_over;
	//short x, y;

public:
	std::mutex _sl;
	std::atomic<SESSION_STATE> _s_state;
	int npc_id;
	SOCKET _socket;
	short sectorX, sectorY;	//한 사람이 동시에 두번 움직이진 않고, 남의 sector를 바꿀 일이 없으므로 atomic이 아님
	//char _name[NAME_SIZE];
	int _prev_remain;

	std::unordered_set <int> view_list;
	std::mutex	vl;

	lua_State* L;
	std::mutex	ll;

public:

	SESSION();

	~SESSION();

	void do_recv();

	void do_send(void* packet);

	void send_login_info_packet();
	void send_login_fail_packet(int reason);
	void send_put_packet(int c_id);
	void send_move_packet(int c_id, int client_time);
	void send_remove_packet(int c_id);
	void send_chat_packet(int c_id, const char* mess);
	void send_stat_change_packet(int c_id);


	void setXY(short x, short y);
	void setDamage(int& damage);
	bool appendEXP(int exp);
	void adaptDeath();

	short X();
	short Y();
	void ID(int a);
	int ID();
	void setData(LoginData& d);
	LoginData& getData();
	char* NAME();

	friend int distance_cell(int a, int b);

};