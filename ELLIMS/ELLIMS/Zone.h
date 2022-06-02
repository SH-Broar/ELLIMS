#pragma once
#include "Turboc.h"

enum class FramePrintType { FULL, POINT };
enum class ZoneWrapMode { NOWRAPPING, WRAPPING };

class Zone
{
private:
	int l, r, b, t;
	
	ZoneWrapMode wrappingMode;
	FramePrintType printType;
	char borderCharacter;

	const char* text;
	bool isClickable;
	int (*clickFunction)(int, int);

public:
	Zone();
	Zone(int _l, int _r, int _b, int _t, bool Clickable = true); 

	void setRegion(int _l, int _r, int _b, int _t);
	void setMode(ZoneWrapMode mode);
	void setType(FramePrintType type);
	void setBorder(char border);

	void print();
	bool clicked(int mx, int my);
	bool Intersect(int _x, int _y);

	void operator= (const char* data);
	void operator= (int (*cFunction)(int, int));
};

