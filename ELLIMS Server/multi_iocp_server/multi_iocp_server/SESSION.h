#pragma once
#include "AMBIT.h"
#include "main.h"

enum SESSION_STATE;
enum COMP_TYPE;

class OVER_EXP
{
public:
	WSAOVERLAPPED _over;	//Ŭ���� ��� �ּҿ� ���� ��ġ�� �ּҸ� �������� �ϱ� ���� �� ó���� ��
							//������ map ��ߵǴµ� �ʹ� �����ϰ� ������嵵 ũ�� send�� ���浵 ���� �ֱ⶧���� �׳� �̷��� �ϼ�
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
	short sectorX, sectorY;	//�� ����� ���ÿ� �ι� �������� �ʰ�, ���� sector�� �ٲ� ���� �����Ƿ� atomic�� �ƴ�
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