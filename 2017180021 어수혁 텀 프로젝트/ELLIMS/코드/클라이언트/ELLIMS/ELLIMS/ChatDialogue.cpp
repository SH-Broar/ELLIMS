#include "Turboc.h"
#include "ChatDialogue.h"
#include "Zone.h"

ChatDialogue::ChatDialogue()
{
	dialogueZone = nullptr;
	currented = 0;
	for (int i = 0; i < 1000; i++)
	{
		messages[i] = ' ';
	}
	messages[816] = '\0';
}

void ChatDialogue::setZone(Zone* z)
{
	dialogueZone = z;
}

void ChatDialogue::dialoguePrint(char* mess)
{
	std::string tmp{ messages + dialogueZone->getWidth() };
	tmp += mess;
	
	for (int i = 0; i < dialogueZone->getWidth() - strlen(mess); i++)
	{
		tmp += ' ';
	}

	*dialogueZone = tmp.c_str();
}

void ChatDialogue::dialoguePrint(std::string mess)
{
	std::string tmp{ messages + dialogueZone->getWidth() };
	tmp += mess;

	for (int i = 0; i < dialogueZone->getWidth() - mess.length(); i++)
	{
		tmp += ' ';
	}

	strcpy(messages, tmp.c_str());
	*dialogueZone = messages;

}

