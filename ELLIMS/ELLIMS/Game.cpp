#include "Game.h"

bool Game::gameEnd = false;


int Game::GameStart()
{
	//
	while (true)
	{
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
		SleepEx(25, TRUE);
	}

	gameEnd = true;
	return 0;
}