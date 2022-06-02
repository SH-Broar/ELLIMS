#include <iostream>
#include "Game.h"

int Game::mouse_X = 0;
int Game::mouse_Y = 0;

std::atomic<bool> Game::mouse_Left_down;
std::atomic<bool> Game::mouse_Left_down_Event;

void Game::MouseClick()
{
	bool eventOnce = false;

	HANDLE       hIn, hOut;
	DWORD        dwNOER;
	INPUT_RECORD rec;

	hIn = GetStdHandle(STD_INPUT_HANDLE);
	hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleMode(hIn, ENABLE_PROCESSED_INPUT | ENABLE_MOUSE_INPUT);

	while (TRUE) {
		if (gameEnd)
			break;
		ReadConsoleInput(hIn, &rec, 1, &dwNOER);

		if (rec.EventType == MOUSE_EVENT)
		{
			mouse_X = rec.Event.MouseEvent.dwMousePosition.X;
			mouse_Y = rec.Event.MouseEvent.dwMousePosition.Y;

			if (rec.Event.MouseEvent.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) {
				mouse_Left_down = true;
				if (!eventOnce)
				{
					mouse_Left_down_Event = true;
					eventOnce = true;
				}

				continue;
			}
			mouse_Left_down = false;
			eventOnce = false;
		}
		SleepEx(25, TRUE);

	}
}

COORD Game::getMouseXY()
{
	return { static_cast<short>(mouse_X),static_cast<short>(mouse_Y) };
}

