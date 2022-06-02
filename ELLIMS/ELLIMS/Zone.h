#pragma once
#include "Turboc.h"

enum class FramePrintType { NONE, FULL, POINT };
enum class ZoneWrapMode { NOWRAPPING, WRAPPING };
enum class ClickableType { NONE, HOVER, BUTTON };

class Zone
{
private:
	int l, r, b, t;
	
	ZoneWrapMode wrappingMode;
	FramePrintType printType;
	char borderCharacter;

	const char* text;
	ClickableType isClickable;
	bool hovered;
	std::function<int(int,int)> CallbackFunction;

public:
	Zone();
	Zone(int _l, int _r, int _b, int _t, ClickableType Clickable = ClickableType::NONE);

	void setRegion(int _l, int _r, int _b, int _t);
	void setMode(ZoneWrapMode mode);
	void setType(FramePrintType type);
	void setClickableType(ClickableType type);
	void setBorder(char border);

	void print();
	bool MouseInteraction(int mx, int my, bool clicked);
	bool Intersect(int _x, int _y);

	void operator= (const char* data);
	void operator= (std::function<int(int, int)>);
};

