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
	static std::array<Player*, 5000> players;
	static int nowPlayerNums;


	//-------외부
private:
	static int mouse_X;
	static int mouse_Y;
	static char inputChar;
	static std::atomic<bool> mouse_Left_down;
	static std::atomic<bool> mouse_Left_down_Event;
	static std::atomic<bool> newCharInputed;
	static std::atomic<PlayerCommand> newCommandInputed;

public:
	static void MouseClick();
	static COORD getMouseXY();

	static char FrameBuffer[120][35];
	static char DoubleFrameBuffer[120][35];
	static char DebugFrameBuffer[120][35];
	static ChatDialogue chat;

	static void print(const char* data, int x, int y);

	//---------네트워크
public:
	//static Network network;
	static bool isGameEnded();
	static void printDebug(const char* data, const char* name = nullptr);
	static void clearDebug();
	static void networkT();

	static bool networkConnected;
	static bool ingame;

	//----
private:
	static int debugNum;
	static bool debugConsole;
	static bool debugKeyInput;

};

