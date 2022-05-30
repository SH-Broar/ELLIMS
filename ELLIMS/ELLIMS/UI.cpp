#include <iostream>
#include "Game.h"

char Game::FrameBuffer[120][35] = {};
char Game::DoubleFrameBuffer[120][35] = {};

std::atomic<bool> Game::mouse_Left_down;

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
				if (FrameBuffer[i][j] != DoubleFrameBuffer[i][j])
				{
					FrameBuffer[i][j] = DoubleFrameBuffer[i][j];
					gotoxy(i, j);
					std::cout << DoubleFrameBuffer[i][j];
				}
			}
		}
		SleepEx(25, TRUE);
	}
}

void Game::print(const char* data, int x, int y)
{
	if (SCREEN_WIDTH - x > strlen(data))
	{
		strcpy_s(&DoubleFrameBuffer[x][y], SCREEN_WIDTH - x, data);
	}
}