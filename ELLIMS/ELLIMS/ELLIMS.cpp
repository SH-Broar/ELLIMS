#include <thread>
#include "Game.h"

Game game{};

int main()
{
	system("mode con cols=120 lines=35");
	SetConsoleTitle(L"E L L I M S");
	std::thread mouseThread{ game.MouseClick };
	std::thread printThread{ game.InitUISetting };
	std::thread networkThread{ game.networkT };

	game.GameStart();


	printf("Bye bye!");
	SleepEx(10000, TRUE);

	networkThread.join();
	mouseThread.join();
	printThread.join();
}