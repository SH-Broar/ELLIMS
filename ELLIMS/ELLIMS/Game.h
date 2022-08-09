#pragma once
#include "Turboc.h"

class Zone;
class Player;
class Scene;
class ChatDialogue;

enum class ClearType;

class Game
{
private:
	static bool gameEnd;

	static Scene nowScene;
	static Zone* focusedZone;

public:
	Game();

	static void InitUISetting();

	int GameStart();

	static void GameFrameAdvance();
	static void gameEnded();

	static void setFocusZone(Zone& z, ClearType c);
	static void setFocusZone(Zone* z);
	static bool isthisFocused(Zone& z);

	static int myPlayerID;
	static std::unordered_map<int, int> playerIDMapper;
	static std::array<Player*, (MAX_USER+NUM_NPC)*3> players;
	static int nowPlayerNums;


	//-------외부
private:
	static int mouse_X;
	static int mouse_Y;
	static char inputChar;
	static std::atomic<bool> mouse_Left_down;
	static std::atomic<bool> mouse_Left_down_Event;
	static std::atomic<bool> newCharInputed;
	static std::atomic<bool> oncedPress;
	static std::atomic<PlayerCommand> newCommandInputed;

	static HANDLE       hIn, hOut;
	static DWORD        dwNOER;
	static INPUT_RECORD rec;

public:
	static void PrepareMouseAndKeyboardModule();
	static void MouseAndKeyboardEvent();
	static COORD getMouseXY();

	static std::string FrameBuffer[SCREEN_HEIGHT];
	static char DoubleFrameBuffer[SCREEN_WIDTH][SCREEN_HEIGHT];
	static char DebugFrameBuffer[SCREEN_WIDTH][SCREEN_HEIGHT];
	static ChatDialogue chat;
	static std::atomic<bool> FrameChanged;

	static void print(const char* data, int x, int y);

	//---------네트워크
public:
	//static Network network;
	static bool isGameEnded();
	static void printDebug(const char* data, const char* name = nullptr);
	static void clearDebug();
	static void networkT();

	static bool networkConnected;
	static bool DBConnected;
	static bool ingame;

	static bool monsterStatus;

	static char addr[20];
	//----
private:
	static int debugNum;
	static bool debugConsole;
	static bool debugKeyInput;


};

