#pragma once
#include "Turboc.h"

class Zone;

class ChatDialogue
{
public:
	ChatDialogue();

	void dialoguePrint(char* mess);

private:
	Zone* dialogueZone;

	std::vector<char*> messages;

};

