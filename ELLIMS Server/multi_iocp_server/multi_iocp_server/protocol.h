constexpr int PORT_NUM = 4000;

constexpr int BUF_SIZE = 200;
constexpr int MESS_SIZE = 128;
constexpr int NAME_SIZE = 20;

constexpr int W_WIDTH = 300;	//나중에 2000으로
constexpr int W_HEIGHT = 300;

constexpr int MAX_USER = 100000;
constexpr int NUM_NPC = 1000; //나중에 20만으로

// Packet ID
constexpr char CS_LOGIN = 0;
constexpr char CS_MOVE = 1;
constexpr char CS_ATTACK = 2;
constexpr char CS_CHAT = 3;

constexpr char SC_LOGIN_OK = 11;
constexpr char SC_LOGIN_FAIL = 12;
constexpr char SC_ADD_OBJECT = 13;
constexpr char SC_REMOVE_OBJECT = 14;
constexpr char SC_MOVE_OBJECT = 15;
constexpr char SC_CHAT = 16;
constexpr char SC_STAT_CHANGE = 17;

#pragma pack (push, 1)
struct CS_LOGIN_PACKET {
	unsigned char size;
	char	type;
	bool  isNewUser;
	char	name[NAME_SIZE];
	char  pass[NAME_SIZE];
};

struct CS_MOVE_PACKET {
	unsigned char size;
	char	type;
	char	direction;  // 0 : UP, 1 : DOWN, 2 : LEFT, 3 : RIGHT
	unsigned  client_time;
};

struct CS_ATTACK_PACKET {
	unsigned char size;
	char	type;
	char  skilltype;
};

struct CS_CHAT_PACKET
{
	unsigned char size;
	char	type;
	int		target_id;
	char	chat_type;			// 0 : say,  1 : tell, 2 : shout
	char	mess[MESS_SIZE];
};

//ok
struct SC_LOGIN_OK_PACKET {
	unsigned char size;
	char	type;
	int 	id;
	short race;
	short x, y;
	short level;
	int	 EXP;
	int HP, MaxHP;
	int MP, MaxMP;
	char	name[NAME_SIZE];
};

//reason
struct SC_LOGIN_FAIL_PACKET {
	unsigned char size;
	char	type;
	int		reason;				// 0 : Invalid Name  (특수문자, 공백 제외)
								// 1 : Name Already Playing
								// 2 : Server Full
};

//struct SC_LOGIN_INFO_PACKET {
//	unsigned char size;
//	char	type;
//	int 	id;
//	char	name[NAME_SIZE];
//	int level;
//	short	x, y;
//	int HP, MaxHP;
//	int MP, MaxMP;
//	int EXP;
//};


struct SC_ADD_OBJECT_PACKET {
	unsigned char size;
	char	type;
	int		id;
	short	x, y;
	short	race;			// 종족 : 인간, 엘프, 드워프, 오크, 드래곤
							// 클라이언트에서 종족별로 별도의 그래픽 표현
							// 추가적으로 성별이나, 직업을 추가할 수 있다.
	char	name[NAME_SIZE];
	short	level;
	int		hp, hpmax;
	int		mp, mpmax;
};

//struct SC_ADD_PLAYER_PACKET {
//	unsigned char size;
//	char	type;
//	int		id;
//	char	name[NAME_SIZE];
//	short	x, y;
//
//};

struct SC_REMOVE_OBJECT_PACKET {
	unsigned char size;
	char	type;
	int		id;
};

//struct SC_REMOVE_PLAYER_PACKET {
//	unsigned char size;
//	char	type;
//	int		id;
//};

struct SC_MOVE_OBJECT_PACKET {
	unsigned char size;
	char	type;
	int		id;
	short	x, y;
	unsigned int client_time;
};

//struct SC_MOVE_PLAYER_PACKET {
//	unsigned char size;
//	char	type;
//	int		id;
//	short	x, y;
//	unsigned int client_time;
//};

struct SC_CHAT_PACKET {
	unsigned char size;
	char	type;
	int		id;
	char	chat_type;		// 0 : 일반 채팅 (say)
						// 1 : 1:1 통신 (tell)
						// 2 : 방송 (shout)
						// 3 : 공지
	char	name[NAME_SIZE];
	char	mess[MESS_SIZE];
};

struct SC_STAT_CHANGE_PACKET {
	unsigned char size;
	char	type;
	int		id;
	short	level;
	int		exp;
	int		hp, hpmax;
	int		mp, mpmax;
};

#pragma pack (pop)