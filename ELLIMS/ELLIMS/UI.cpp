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

void Game::print(const char* data, Zone& z)
{
	int trackerX = z.l;
	int trackerY = z.b;

	const char* tmp = data;
	while (*tmp != '\0')
	{
		DoubleFrameBuffer[trackerX][trackerY] = *tmp;
		trackerX++;

		if (trackerX - z.l > z.r - z.l)
		{
			trackerX = z.l;
			trackerY++;

			if (trackerY - z.b > z.t - z.b && z.printMode == ZoneWrapMode::WRAPPING)
			{
				break;
			}
		}

		tmp = tmp + 1;
	}
}

void Game::print(Zone& z, FramePrintType type, char p)
{
	int x = z.l - 1;
	int y = z.b - 1;

	int mx = z.r + 1;
	int my = z.t + 1;

	for (int i = x; i <= mx; i++)
	{
		for (int j = y; j <= my; j++)
		{
			if (i >= 0 && i < SCREEN_WIDTH)
			{
				if (j >= 0 && j < SCREEN_HEIGHT)
				{
					if (type == FramePrintType::FULL)
					{
						if (i == x || i == mx || j == y || j == my)
						{
							DoubleFrameBuffer[i][j] = p;
						}
					}
					else if (type == FramePrintType::POINT)
					{
						if ((i == x || i == mx) && (j == y || j == my))
						{
							DoubleFrameBuffer[i][j] = p;
						}
					}
				}
			}
		}
	}
	
}