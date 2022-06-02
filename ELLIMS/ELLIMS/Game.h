#pragma once
#include <atomic>
#include "Scene.h"
#include "Turboc.h"

class Game
{
private:
	static bool gameEnd;
	Scene nowScene;

public:
	Game();

	static void InitUISetting();

	int GameStart();
	static void GameFrameAdvance();
	static void gameEnded();


	//-------¿ÜºÎ
private:
	static int mouse_X;
	static int mouse_Y;
	static std::atomic<bool> mouse_Left_down;
	static std::atomic<bool> mouse_Left_down_Event;

public:
	static void MouseClick();
	static COORD getMouseXY();

	static char FrameBuffer[120][35];
	static char DoubleFrameBuffer[120][35];

	void print(const char* data, int x, int y);

};

