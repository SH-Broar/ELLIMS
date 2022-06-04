#pragma once
#include <atomic>
#include "Scene.h"
#include "Network.h"
#include "Turboc.h"
#include "Player.h"

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
	void PacketProcess();

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

	static void print(const char* data, int x, int y);

	//---------네트워크
	static Network network;
	static bool isGameEnded();
};

