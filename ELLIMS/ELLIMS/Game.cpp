#include <thread>

#include "Game.h"
#include "Player.h"
#include "Scene.h"


char Game::inputChar;
bool Game::networkConnected = false;
bool Game::gameEnd = false;
Zone* Game::focusedZone = nullptr;
std::atomic<bool> Game::newCharInputed;
std::atomic<PlayerCommand> Game::newCommandInputed;
bool Game::ingame;
int Game::nowPlayerNums = 0;

int Game::GameStart()
{
	//여기서 초기화
	clearDebug();

	SleepEx(25, TRUE);
	nowScene.changeScene(SceneName::TITLE);

	while (true)
	{
		//hover and click
		nowScene.clickScene(mouse_X, mouse_Y, mouse_Left_down_Event);

		if (debugConsole)
		{
			nowScene.printDebugConsole();
		}
		else
		{
			nowScene.printScene();
		}
		if (newCharInputed)
		{
			if (focusedZone != nullptr)
				focusedZone->addTyping(inputChar);
		}
		if (ingame)
		{
			//Game::printDebug("LOOP");
			nowScene.UpdateScene(*players[playerIDMapper[myPlayerID]]);

			for (int i = 0; i < nowPlayerNums; i++)
			{
				if (players[i] != nullptr)
				{
					players[i]->MouseInteraction(mouse_X, mouse_Y, mouse_Left_down_Event);
					players[i]->SetPlayersRegion(players[playerIDMapper[myPlayerID]]->x, players[playerIDMapper[myPlayerID]]->y);
					players[i]->print();
				}
			}
			if (newCommandInputed != PlayerCommand::NONE)
			{
				players[playerIDMapper[myPlayerID]]->ProcessCommand(newCommandInputed);
			}
		}


		if (gameEnd)
			break;
		mouse_Left_down_Event = false;
		newCharInputed = false;
		newCommandInputed = PlayerCommand::NONE;
		SleepEx(10, TRUE);
	}
	return 0;
}

void Game::setFocusZone(Zone& z, ClearType c)
{
	z.clearZone(c);
	z = " ";

	focusedZone = &z;
}

void Game::networkT()
{
	while (!Game::networkConnected) { SleepEx(30, TRUE); }

	while (networkConnected)
	{
		Network::PacketProcess();

		if (gameEnd)
			break;
	}
}

bool Game::isGameEnded()
{
	return gameEnd;
}
