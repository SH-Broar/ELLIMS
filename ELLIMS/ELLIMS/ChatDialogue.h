#pragma once
#include "Turboc.h"

class Zone;

class ChatDialogue
{
public:
	ChatDialogue();

	void setZone(Zone* z);
	void dialoguePrint(char* mess);
	void dialoguePrint(std::string mess);
private:
	Zone* dialogueZone;

	char messages[1000]{};
	int currented;
};

