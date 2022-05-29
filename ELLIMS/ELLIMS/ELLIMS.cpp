#include <thread>
#include "Game.h"

Game game{};

int main()
{
	system("mode con cols=120 lines=35");
	SetConsoleTitle(L"E L L I M S");
	std::thread mouseThread{ game.MouseClick };
	std::thread printThread{ game.InitUISetting };

	game.GameStart();

	mouseThread.join();
	printThread.join();
}