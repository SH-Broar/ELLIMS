#include "Game.h"

bool Game::gameEnd = false;


int Game::GameStart()
{
	//
	while (true)
	{
		SleepEx(25,TRUE);
		if (mouse_Left_down)
		{
			print("*", mouse_X, mouse_Y);
		}
		//contents
		// �� ���
		// ���� ���
		// ä�� ���

		//CClear();
		//std::cout << mouse_X;

		//break;
	}

	gameEnd = true;
	return 0;
}