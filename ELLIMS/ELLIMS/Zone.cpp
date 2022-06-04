#include "Zone.h"
#include "Game.h"

Zone::Zone() : l(0), r(0), t(0), b(0), wrappingMode(ZoneWrapMode::WRAPPING), printType(FramePrintType::NONE), hovered(false), borderCharacter('*'), text(""), isClickable(ClickableType::NONE), CallbackFunction(nullptr), isActive(true), temperanceActive(true) {};
Zone::Zone(int _l, int _r, int _b, int _t, ClickableType Clickable) : l(_l), r(_r), b(_b), t(_t), isClickable(Clickable), hovered(false), wrappingMode(ZoneWrapMode::WRAPPING), printType(FramePrintType::NONE), borderCharacter('*'), text(""), CallbackFunction(nullptr), isActive(true), temperanceActive(true)
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
	strcpy(text, data);
}

void Zone::operator=(std::function<int(int, int)> func)
{
	CallbackFunction = func;
}

void Zone::addTyping(char c)
{
	strcat(text, &c);
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
									Game::DoubleFrameBuffer[i][j] = ' ';
								}
							}
							else if (printType == FramePrintType::POINT)
							{
								if ((i == x || i == mx) && (j == y || j == my))
								{
									Game::DoubleFrameBuffer[i][j] = ' ';
								}
							}
						}
					}
				}
			}
		}
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
			if (CallbackFunction != nullptr)
			{
				int ret = CallbackFunction(mx, my);

			}
			return true;
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


