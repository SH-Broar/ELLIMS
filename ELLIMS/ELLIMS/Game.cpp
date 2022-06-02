#include "Game.h"

bool Game::gameEnd = false;


int Game::GameStart()
{
	//여기서 초기화

	while (true)
	{
		if (mouse_Left_down_Event)
		{
			//여기서 Scene의 모든 zone에 clicked 실행
		}

		if (mouse_Left_down_Event)
		{
			Zone tmpz{ mouse_X,mouse_X + 9,mouse_Y,mouse_Y + 1,true };
			tmpz = "1234567890123456789012345678901234567890";
			tmpz.print();
		}
		//contents
		// 맵 출력
		// 스탯 출력
		// 채팅 출력

		//CClear();
		//std::cout << mouse_X;

		//break;
		SleepEx(25, TRUE);
	}

	gameEnd = true;
	return 0;
}