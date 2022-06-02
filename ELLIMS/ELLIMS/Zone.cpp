#include "Zone.h"
#include "Game.h"

Zone::Zone(): l(0), r(0), t(0), b(0), wrappingMode(ZoneWrapMode::WRAPPING), printType(FramePrintType::FULL), borderCharacter('*'),text(nullptr),isClickable(false),clickFunction(nullptr) {};
Zone::Zone(int _l, int _r, int _b, int _t, bool Clickable) : l(_l), r(_r), b(_b), t(_t), isClickable(Clickable), wrappingMode(ZoneWrapMode::WRAPPING), printType(FramePrintType::FULL), borderCharacter('*'), text(nullptr), clickFunction(nullptr)
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
void Zone::setBorder(char border) 
{
	borderCharacter = border;
}

void Zone::operator=(const char* data)
{
	text = data;
}

void Zone::operator=(int(*cFunction)(int, int))
{
	clickFunction = cFunction;
}

void Zone::print()
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

	int trackerX = l;
	int trackerY = b;

	const char* tmp = text;
	while (*tmp != '\0')
	{
		Game::DoubleFrameBuffer[trackerX][trackerY] = *tmp;
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

		tmp = tmp + 1;
	}

}

bool Zone::clicked(int mx, int my)
{
	if (isClickable && Intersect(mx, my))
	{
		if (clickFunction != nullptr)
		{
			clickFunction(mx, my);
		}
		return true;
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


