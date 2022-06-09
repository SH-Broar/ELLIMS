constexpr int PORT_NUM = 4000;
constexpr char* SERVER_ADDR = (char*)"127.0.0.1";

constexpr int BUF_SIZE = 128;
constexpr int MESS_SIZE = 128;
constexpr int NAME_SIZE = 20;

constexpr int W_WIDTH = 400;
constexpr int W_HEIGHT = 400;

constexpr int MAX_USER = 1000;
constexpr int NUM_NPC = 20000;

// Packet ID
constexpr char CS_LOGIN = 0;
constexpr char CS_MOVE = 1;
constexpr char CS_CHAT = 2;

constexpr char SC_LOGIN_INFO = 3;
constexpr char SC_ADD_PLAYER = 4;
constexpr char SC_REMOVE_PLAYER = 5;
constexpr char SC_MOVE_PLAYER = 6;
constexpr char SC_CHAT = 7;

#pragma pack (push, 1)
struct CS_LOGIN_PACKET {
	unsigned char size;
	char	type;
	char	name[NAME_SIZE];
	char  pass[NAME_SIZE];
};

struct CS_MOVE_PACKET {
	unsigned char size;
	char	type;
	char	direction;  // 0 : UP, 1 : DOWN, 2 : LEFT, 3 : RIGHT
	unsigned  client_time;
};

struct CS_CHAT_PACKET
{
	unsigned char size;
	char	type;
	char	mess[MESS_SIZE];
};

struct SC_LOGIN_INFO_PACKET {
	unsigned char size;
	char	type;
	int 	id;
	char	name[NAME_SIZE];
	int level;
	short	x, y;
	int HP, MaxHP;
	int MP, MaxMP;
	int EXP;
};

struct SC_ADD_PLAYER_PACKET {
	unsigned char size;
	char	type;
	int		id;
	char	name[NAME_SIZE];
	short	x, y;

};

struct SC_REMOVE_PLAYER_PACKET {
	unsigned char size;
	char	type;
	int		id;
};

struct SC_MOVE_PLAYER_PACKET {
	unsigned char size;
	char	type;
	int		id;
	short	x, y;
	unsigned int client_time;
};

struct SC_CHAT_PACKET {
	unsigned char size;
	char	type;
	int		id;
	char	name[NAME_SIZE];
	char	mess[MESS_SIZE];
};

#pragma pack (pop)