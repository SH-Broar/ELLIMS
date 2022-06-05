#pragma once
#include "Turboc.h"

class Game;

enum class FramePrintType { NONE, FULL, POINT };
enum class ZoneWrapMode { NOWRAPPING, WRAPPING };
enum class ClickableType { NONE, HOVER, BUTTON };
enum class ClearType { NONE, TEXT, FULL };

class Zone
{
protected:
	int l, r, b, t;
	bool temperanceActive;
	bool isActive;
	
	ZoneWrapMode wrappingMode;
	FramePrintType printType;
	char borderCharacter;

	char text[SCREEN_WIDTH*SCREEN_HEIGHT];
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
	void setActive(bool active);
	void setActiveByFrame();

	void operator= (const char* data);
	void operator= (std::function<int(int, int)>);

	void addTyping(char c);

	virtual void print();
	void clearZone(ClearType c);
	bool MouseInteraction(int mx, int my, bool clicked);
	bool Intersect(int _x, int _y);




};

