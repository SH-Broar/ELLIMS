#pragma once
#include <atomic>
#include "Scene.h"
#include "Network.h"
#include "Player.h"
#include "Turboc.h"


class Game
{
private:
	static bool gameEnd;

	Scene nowScene;
	static Zone* focusedZone;

	Player player;

public:
	Game();

	static void InitUISetting();

	int GameStart();

	static void GameFrameAdvance();
	static void gameEnded();

	static void setFocusZone(Zone& z, ClearType c);


	//-------외부
private:
	static int mouse_X;
	static int mouse_Y;
	static char inputChar;
	static std::atomic<bool> mouse_Left_down;
	static std::atomic<bool> mouse_Left_down_Event;
	static std::atomic<bool> newCharInputed;

public:
	static void MouseClick();
	static COORD getMouseXY();

	static char FrameBuffer[120][35];
	static char DoubleFrameBuffer[120][35];
	static char DebugFrameBuffer[120][35];

	static void print(const char* data, int x, int y);

	//---------네트워크
public:
	//static Network network;
	static bool isGameEnded();
	static void printDebug(const char* data, const char* name = nullptr);
	static void clearDebug();

	static bool networkConnected;

	//----
private:
	static int debugNum;
	static bool debugConsole;

};

