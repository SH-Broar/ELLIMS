#include "Turboc.h"
#include "Game.h"
#include "Player.h"
#include "Scene.h"
#include "Zone.h"


#define DEBUG

std::string Game::FrameBuffer[SCREEN_HEIGHT] = {};
char Game::DoubleFrameBuffer[SCREEN_WIDTH][SCREEN_HEIGHT] = {};
char Game::DebugFrameBuffer[SCREEN_WIDTH][SCREEN_HEIGHT] = {};

std::atomic<bool> Game::FrameChanged = true;

int Game::debugNum = 0;

Scene Game::nowScene;
int Game::myPlayerID;
std::unordered_map<int, int>  Game::playerIDMapper;
std::array<Player*, (MAX_USER + NUM_NPC) * 3>  Game::players;

Game::Game()
{
}

void Game::InitUISetting()
{
	setcursortype(CURSOR_TYPE::NOCURSOR);

	for (int j = 0; j < SCREEN_HEIGHT; j++)
	{
		for (int i = 0; i < SCREEN_WIDTH; i++)
		{
			
			//FrameBuffer[i][j] = ' ';
			DoubleFrameBuffer[i][j] = ' ';
		}
		FrameBuffer[j].resize(SCREEN_WIDTH,'\0');
	}

	GameFrameAdvance();
}

void Game::GameFrameAdvance()
{
	while (true)
	{
		if (gameEnd)
			break;
		if (!FrameChanged)
		{
			SleepEx(50, TRUE);
			continue;
		}

		for (int j = 0; j < SCREEN_HEIGHT; j++)
		{
			for (int i = 0; i < SCREEN_WIDTH; i++)
			{
#ifdef DEBUG
				if (debugConsole)
				{
					if (FrameBuffer[j].at(i) != DebugFrameBuffer[i][j])
					{
						FrameBuffer[j].at(i) = DebugFrameBuffer[i][j];
						//FrameBuffer[j].replace(i, 1, &DebugFrameBuffer[i][j]);
						//gotoxy(i, j);
						//std::cout << DebugFrameBuffer[i][j];
					}
				}
				else
				{
					if (FrameBuffer[j].at(i) != DoubleFrameBuffer[i][j])
					{
						FrameBuffer[j].at(i) = DoubleFrameBuffer[i][j];
						//FrameBuffer[j].replace(i, 1, &DoubleFrameBuffer[i][j]);
						//FrameBuffer[i][j] = DoubleFrameBuffer[i][j];
						//gotoxy(i, j);
						//std::cout << DoubleFrameBuffer[i][j];
					}
				}
#else
				if (FrameBuffer[j].at(i) != DoubleFrameBuffer[i][j])
				{
					FrameBuffer[j].at(i) = DoubleFrameBuffer[i][j];
					//FrameBuffer[j].replace(i, 1, &DoubleFrameBuffer[i][j]);
					//FrameBuffer[i][j] = DoubleFrameBuffer[i][j];
					//gotoxy(i, j);
					//std::cout << DoubleFrameBuffer[i][j];
				}
#endif
			}
			
		}
		gotoxy(0, 0);
		for (int j = 0; j < SCREEN_HEIGHT; j++)
		{
			std::cout << FrameBuffer[j];
		}

		FrameChanged = false;
		SleepEx(50, TRUE);
	}
}

void Game::gameEnded()
{
	//std::cout << "end";
	printf("Check F1");
	SleepEx(5000, TRUE);
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
	FrameChanged = true;
}

void Game::printDebug(const char* data, const char* name)
{
#ifdef DEBUG
	int trackerX = 5 + (debugNum % 64) / (SCREEN_HEIGHT / 2 - 1) * 30;
	int trackerY = 2 + ((debugNum % 64) % (SCREEN_HEIGHT / 2 - 1)) * 2;

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
#endif
}

void Game::clearDebug()
{
#ifdef DEBUG
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
	Game::FrameChanged = true;
#endif
}