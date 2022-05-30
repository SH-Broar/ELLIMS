#pragma once

enum class ZoneWrapMode {NOWRAPPING, WRAPPING};

class Zone
{
public:
	int l, r, b, t;
	ZoneWrapMode printMode;

public:
	Zone();
	Zone(int _l, int _r, int _b, int _t, ZoneWrapMode mode = ZoneWrapMode::WRAPPING); 

	bool Intersect(int _x, int _y);
};

