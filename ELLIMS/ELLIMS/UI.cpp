#include <iostream>
#include "Game.h"

char Game::FrameBuffer[120][35] = {};
char Game::DoubleFrameBuffer[120][35] = {};
char Game::DebugFrameBuffer[120][35] = {};

int Game::debugNum = 0;

Game::Game()
{
}

void Game::InitUISetting()
{
	setcursortype(CURSOR_TYPE::NOCURSOR);

	for (int i = 0; i < SCREEN_WIDTH; i++)
	{
		for (int j = 0; j < SCREEN_HEIGHT; j++)
		{
			FrameBuffer[i][j] = ' ';
			DoubleFrameBuffer[i][j] = ' ';
		}
		std::cout << FrameBuffer[i];
	}
	
	GameFrameAdvance();
}

void Game::GameFrameAdvance()
{
	while (true)
	{
		if (gameEnd)
			break;
		for (int i = 0; i < SCREEN_WIDTH; i++)
		{
			for (int j = 0; j < SCREEN_HEIGHT; j++)
			{
				if (debugConsole)
				{
					if (FrameBuffer[i][j] != DebugFrameBuffer[i][j])
					{
						FrameBuffer[i][j] = DebugFrameBuffer[i][j];
						gotoxy(i, j);
						std::cout << DebugFrameBuffer[i][j];
					}
				}
				else
				{
					if (FrameBuffer[i][j] != DoubleFrameBuffer[i][j])
					{
						FrameBuffer[i][j] = DoubleFrameBuffer[i][j];
						gotoxy(i, j);
						std::cout << DoubleFrameBuffer[i][j];
					}
				}
			}
		}
		SleepEx(25, TRUE);
	}
}

void Game::gameEnded()
{
	//std::cout << "end";
	gameEnd = true;
}


void Game::print(const char* data, int x, int y)
{
	int trackerX = x;
	int trackerY = y;

	int tracs = 0;
	while (strlen(data) > tracs)
	{
		Game::DoubleFrameBuffer[trackerX][trackerY] = data[tracs];
		trackerX++;

		tracs++;
	}

}

void Game::printDebug(const char* data, const char* name)
{
	int trackerX = 5 + debugNum/(SCREEN_HEIGHT / 2 - 1) * 30;
	int trackerY = 2 + (debugNum%(SCREEN_HEIGHT / 2 - 1))*2;

	int tracs = 0;

	char tmp[SCREEN_WIDTH];

	if (name != nullptr)
	{
		sprintf(tmp, "%d", debugNum);
		strcat(tmp, ". ");
		strcat(tmp, name);
		strcat(tmp, " : ");
		strcat(tmp, data);
	}
	else
	{
		sprintf(tmp, "%d", debugNum);
		strcat(tmp, ". ");
		strcat(tmp, data);
	}

	while (strlen(tmp) > tracs)
	{
		Game::DebugFrameBuffer[trackerX][trackerY] = tmp[tracs];
		trackerX++;

		tracs++;
	}

	debugNum++;

}

void Game::clearDebug()
{
	for (int i = 0; i < SCREEN_WIDTH; ++i)
	{
		for (int j = 0; j < SCREEN_HEIGHT; ++j)
		{
			Game::DebugFrameBuffer[i][j] = ' ';
		}
	}

	constexpr char dg[11] = "DEBUG PAGE";
	for (int i = 0; i < 11; ++i)
	{
		Game::DebugFrameBuffer[i][0] = dg[i];
	}

	debugNum = 0;
}