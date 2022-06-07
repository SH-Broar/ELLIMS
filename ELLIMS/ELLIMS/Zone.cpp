#include "Zone.h"
#include "Game.h"
#include <string>
Zone::Zone() : l(1), r(SCREEN_WIDTH-2), t(SCREEN_HEIGHT-2), b(1), wrappingMode(ZoneWrapMode::WRAPPING), printType(FramePrintType::NONE), hovered(false), borderCharacter('*'), text(""), isClickable(ClickableType::NONE), ClickCallbackFunction(nullptr), isActive(true), temperanceActive(true) {};
Zone::Zone(int _l, int _r, int _b, int _t, ClickableType Clickable) : l(_l), r(_r), b(_b), t(_t), isClickable(Clickable), hovered(false), wrappingMode(ZoneWrapMode::WRAPPING), printType(FramePrintType::NONE), borderCharacter('*'), text(""), ClickCallbackFunction(nullptr), isActive(true), temperanceActive(true)
{};


void Zone::setRegion(int _l, int _r, int _b, int _t)
{
	l = _l;
	r = _r;
	b = _b;
	t = _t;
};

void Zone::setMode(ZoneWrapMode mode)
{
	wrappingMode = mode;
}
void Zone::setType(FramePrintType type)
{
	printType = type;
}
void Zone::setClickableType(ClickableType type)
{
	isClickable = type;
}
void Zone::setBorder(char border)
{
	borderCharacter = border;
}

void Zone::setActive(bool active)
{
	temperanceActive = active;
}

void Zone::setActiveByFrame()
{
	isActive = temperanceActive;
}

void Zone::operator=(const char* data)
{
	strcpy_s(text, data);
}

void Zone::operator=(std::function<int(int, int)> func)
{
	ClickCallbackFunction = func;
}

void Zone::operator=(std::function<void(void)> func)
{
	EnterCallbackFunction = func;
}

void Zone::addTyping(char c)
{
	if (c == '\r')
	{
		if (EnterCallbackFunction != nullptr)
		{
			EnterCallbackFunction();
		}
	}
	else
	{
		std::string t{ c };
		//Game::printDebug(std::to_string(strlen(text)).c_str(), "length");
		strcat_s(text, t.c_str());
	}
}

char* Zone::getText()
{
	if (strlen(text) < MESS_SIZE)
	{
		strcpy(mess, text);
		return mess;
	}
	strcpy(mess, "text TOO Long");
	return mess;
}

void Zone::print()
{
	if (!isActive)
		return;

	int x = l - 1;
	int y = b - 1;

	int mx = r + 1;
	int my = t + 1;

	if (printType != FramePrintType::NONE)
	{
		for (int i = x; i <= mx; i++)
		{
			for (int j = y; j <= my; j++)
			{
				if (i >= 0 && i < SCREEN_WIDTH)
				{
					if (j >= 0 && j < SCREEN_HEIGHT)
					{
						if (printType == FramePrintType::FULL)
						{
							if (i == x || i == mx || j == y || j == my)
							{
								Game::DoubleFrameBuffer[i][j] = borderCharacter;
							}
						}
						else if (printType == FramePrintType::POINT)
						{
							if ((i == x || i == mx) && (j == y || j == my))
							{
								Game::DoubleFrameBuffer[i][j] = borderCharacter;
							}
						}
					}
				}
			}
		}
	}

	int trackerX = l;
	int trackerY = b;

	if (isClickable == ClickableType::HOVER)
	{
		if (hovered)
			Game::DoubleFrameBuffer[trackerX - 2][trackerY] = '>';
		else
			Game::DoubleFrameBuffer[trackerX - 2][trackerY] = ' ';
	}
	
	int tracs = 0;
	while (strlen(text) > tracs)
	{
		Game::DoubleFrameBuffer[trackerX][trackerY] = text[tracs];
		trackerX++;

		if (trackerX - l > r - l)
		{
			trackerX = l;
			trackerY++;

			if (trackerY - b > t - b && wrappingMode == ZoneWrapMode::WRAPPING)
			{
				break;
			}
		}
		tracs++;
	}

	if (isClickable == ClickableType::HOVER)
	{
		if (hovered)
			Game::DoubleFrameBuffer[trackerX + 1][trackerY] = '<';
		else
			Game::DoubleFrameBuffer[trackerX + 1][trackerY] = ' ';
	}

}

void Zone::clearZone(ClearType c)
{
	if (c == ClearType::NONE)
		return;

	if (c == ClearType::FULL)
	{
		int x = l - 1;
		int y = b - 1;

		int mx = r + 1;
		int my = t + 1;

		for (int i = x; i <= mx; i++)
		{
			for (int j = y; j <= my; j++)
			{
				if (i >= 0 && i < SCREEN_WIDTH)
				{
					if (j >= 0 && j < SCREEN_HEIGHT)
					{
						Game::DoubleFrameBuffer[i][j] = ' ';
					}
				}
			}
		}
		return;
	}
	
	int trackerX = l;
	int trackerY = b;

	int tracs = 0;
	while (strlen(text) > tracs)
	{
		Game::DoubleFrameBuffer[trackerX][trackerY] = ' ';
		trackerX++;

		if (trackerX - l > r - l)
		{
			trackerX = l;
			trackerY++;

			if (trackerY - b > t - b && wrappingMode == ZoneWrapMode::WRAPPING)
			{
				break;
			}
		}
		tracs++;
	}

	if (isClickable == ClickableType::HOVER)
	{
		Game::DoubleFrameBuffer[trackerX - 2][trackerY] = ' ';
		Game::DoubleFrameBuffer[trackerX + 1][trackerY] = ' ';
	}
}

bool Zone::MouseInteraction(int mx, int my, bool clicked)
{
	if (isClickable == ClickableType::NONE || !isActive)
		return false;

	bool intered = Intersect(mx, my);

	if (intered)
	{
		if (isClickable == ClickableType::HOVER)
		{
			hovered = true;
		}

		if (clicked)
		{
			if (ClickCallbackFunction != nullptr)
			{
				int ret = ClickCallbackFunction(mx, my);
				return true;
			}
		}
	}
	else
	{
		hovered = false;
	}

	return false;
}
bool Zone::Intersect(int _x, int _y)
{
	if (l <= _x && _x <= r)
	{
		if (b <= _y && _y <= t)
		{
			return true;
		}
	}
	return false;
}


__int64 Zone::getLegion()
{
	if (l >= 1000 || r >= 1000 || b >= 1000 || t >= 1000)
	{
		Game::printDebug("getLegion OVERFLOW", "ZONE");
	}
		
	__int64 t;
	t = l;
	t *= 1000;
	t += r;
	t *= 1000;
	t += b;
	t *= 1000;
	t += t;

	return t;

}

int Zone::getWidth()
{
	return r - l+1;
}