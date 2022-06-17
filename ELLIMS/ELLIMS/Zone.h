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

	//std::string text;
	char text[SCREEN_WIDTH * SCREEN_HEIGHT] = {};
	char mess[MESS_SIZE];

	ClickableType isClickable;
	bool hovered;
	std::function<int(int,int)> ClickCallbackFunction;
	std::function<void(void)> EnterCallbackFunction;

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

	bool ZoneChanged;

	void operator= (const char* data);
	void operator= (std::function<int(int, int)>);
	void operator= (std::function<void(void)>);

	void addTyping(char c);

	virtual void print();
	void clearZone(ClearType c);
	bool MouseInteraction(int mx, int my, bool clicked);
	bool Intersect(int _x, int _y);

	void zoneChanged();

	char* getText();
	int getWidth();

	__int64 getLegion();

};

