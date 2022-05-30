#pragma once
#include <atomic>
#include "Turboc.h"

enum class FramePrintType { FULL, POINT };

class Game
{
private:
	static bool gameEnd;

public:
	Game()
	{
	}

	static void InitUISetting();

	int GameStart();
	static void GameFrameAdvance();



	//-------�ܺ�
private:
	static int mouse_X;
	static int mouse_Y;
	static std::atomic<bool> mouse_Left_down;

	static char FrameBuffer[120][35];
	static char DoubleFrameBuffer[120][35]; 

public:
	static void MouseClick();
	static COORD getMouseXY();

	void print(const char* data, int x, int y);
	void print(const char* data, Zone& z);
	void print(Zone& z, FramePrintType type = FramePrintType::FULL, char p = '*');

};

