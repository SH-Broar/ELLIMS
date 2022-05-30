#include "Zone.h"

Zone::Zone(): l(0), r(0), t(0), b(0), printMode(ZoneWrapMode::WRAPPING) {};
Zone::Zone(int _l, int _r, int _b, int _t, ZoneWrapMode mode) : l(_l), r(_r), b(_b), t(_t)
{
	printMode = mode;
};

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