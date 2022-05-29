#include <iostream>
#include "Game.h"

int Game::mouse_X = 0;
int Game::mouse_Y = 0;

void Game::MouseClick()
{
	while (TRUE)
	{
		HANDLE       hIn, hOut;
		DWORD        dwNOER;
		INPUT_RECORD rec;

		hIn = GetStdHandle(STD_INPUT_HANDLE);
		hOut = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleMode(hIn, ENABLE_PROCESSED_INPUT | ENABLE_MOUSE_INPUT);

		while (TRUE) {
			SleepEx(25, TRUE);
			if (gameEnd)
				break;
			ReadConsoleInput(hIn, &rec, 1, &dwNOER);

			if (rec.EventType == MOUSE_EVENT) {
				if (rec.Event.MouseEvent.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) {
					mouse_Left_down = true;
					mouse_X = rec.Event.MouseEvent.dwMousePosition.X;
					mouse_Y = rec.Event.MouseEvent.dwMousePosition.Y;

					continue;
				}
				mouse_Left_down = false;
			}
		}
	}
}

COORD Game::getMouseXY()
{
	return { static_cast<short>(mouse_X),static_cast<short>(mouse_Y) };
}

