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

int Game::GameStart()
{
	//���⼭ �ʱ�ȭ
	clearDebug();

	SleepEx(25, TRUE);
	nowScene.changeScene(SceneName::TITLE);

	while (true)
	{
		//hover and click
		nowScene.clickScene(mouse_X, mouse_Y, mouse_Left_down_Event);

		if (mouse_Left_down_Event)
		{
			
		}
		if (newCharInputed)
		{
			if (focusedZone != nullptr)
				focusedZone->addTyping(inputChar);
		}
		if (newCommandInputed != PlayerCommand::NONE)
		{
			players[myPlayerID]->ProcessCommand(newCommandInputed);
		}

		// ������Ʈ
		if (ingame)
		{
			//Game::printDebug("LOOP");
			nowScene.UpdateScene(*players[myPlayerID]);
			for (auto op : players)
			{
				op->SetPlayersRegion(players[myPlayerID]->x, players[myPlayerID]->y);
			}
		}

		// �� ���
		if (debugConsole)
		{
			nowScene.printDebugConsole();
		}
		else
		{
			nowScene.printScene();
			if (ingame)
			{
				for (auto pp : players)
				{
					pp->print();
				}
			}
		}

		// ������Ʈ ���

		// ���� ���
		// ä�� ���

		//CClear();
		//std::cout << mouse_X;

		//break;


		if (gameEnd)
			break;
		mouse_Left_down_Event = false;
		newCharInputed = false;
		newCommandInputed = PlayerCommand::NONE;
		SleepEx(25, TRUE);
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
