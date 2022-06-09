#include <iostream>
#include "Game.h"

int Game::mouse_X = 0;
int Game::mouse_Y = 0;

bool Game::debugConsole;
bool Game::debugKeyInput;

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

		if (rec.EventType == KEY_EVENT)
		{
			if (rec.Event.KeyEvent.bKeyDown)
			{
				if (rec.Event.KeyEvent.wVirtualKeyCode)
				{
					switch (rec.Event.KeyEvent.wVirtualKeyCode)
					{
					case VK_F1:
						Game::debugConsole = true;
						break;
					case VK_F5:
						Game::clearDebug();
						break;
					case VK_F12:
						debugKeyInput ^= true;
						break;
					case VK_UP:
						if (debugKeyInput)
							Game::printDebug("UP", "KEY");
						newCommandInputed = PlayerCommand::UP;
						break;
					case VK_DOWN:
						if (debugKeyInput)
							Game::printDebug("DOWN", "KEY");
						newCommandInputed = PlayerCommand::DOWN;
						break;
					case VK_LEFT:
						if (debugKeyInput)
							Game::printDebug("LEFT", "KEY");
						newCommandInputed = PlayerCommand::LEFT;
						break;
					case VK_RIGHT:
						if (debugKeyInput)
							Game::printDebug("RIGHT", "KEY");
						newCommandInputed = PlayerCommand::RIGHT;
						break;
					case VK_ESCAPE:
						if (debugKeyInput)
							Game::printDebug("ESCAPE", "KEY");
						newCommandInputed = PlayerCommand::ESCAPE;
						break;
					case VK_BACK:
						if (debugKeyInput)
							Game::printDebug("BACKSPACE", "KEY");
						newCommandInputed = PlayerCommand::BACKSPACE;
						break;
					default:
						if (!rec.Event.KeyEvent.uChar.AsciiChar)
						{
							if (debugKeyInput)
								Game::printDebug("VIRTUAL", "KEY");
						}
						break;
					}

					if (rec.Event.KeyEvent.uChar.AsciiChar)
					{
						inputChar = rec.Event.KeyEvent.uChar.AsciiChar;
						newCharInputed = true;
						if (debugKeyInput)
							Game::printDebug(&inputChar, "CHAR");
					}
				}
			}
			else if (!rec.Event.KeyEvent.bKeyDown)
			{
				switch (rec.Event.KeyEvent.wVirtualKeyCode)
				{
				case VK_F1:
					Game::debugConsole = false;
					break;
				}
			}
			SleepEx(10, TRUE);
		}
	}
}

COORD Game::getMouseXY()
{
	return { static_cast<short>(mouse_X),static_cast<short>(mouse_Y) };
}
