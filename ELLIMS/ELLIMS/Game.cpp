#include "Game.h"

bool Game::gameEnd = false;


int Game::GameStart()
{
	//
	while (true)
	{
		Zone tmpz{ mouse_X,mouse_X + 9,mouse_Y,mouse_Y + 1,ZoneWrapMode::WRAPPING };

		print(tmpz, FramePrintType::POINT);

		if (mouse_Left_down)
		{
			print("1234567890123456789012345678901234567890", tmpz);
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