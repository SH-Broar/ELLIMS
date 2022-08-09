#include "Turboc.h"
#include "Game.h"

Game game{};
//constexpr char* SERVER_ADDR = (char*)"127.0.0.1";

char Game::addr[20];

int main()
{
	system("mode con cols=120 lines=35");
	SetConsoleTitle(L"E L L I M S");

	std::cout << "IP : ";
	std::cin >> Game::addr;

	game.PrepareMouseAndKeyboardModule();
	std::thread printThread{ game.InitUISetting };
	std::thread networkThread{ game.networkT };

	game.GameStart();


	printf("Bye bye!");
	SleepEx(10000, TRUE);

	networkThread.join();
	printThread.join();
}