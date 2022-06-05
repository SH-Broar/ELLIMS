#pragma once
#include "Turboc.h"
#include "Zone.h"

class Game;
class Network;

class Player : public Zone
{
public:
	Player();

public:
	int pID;
	char characterIcon;
	char name[10];

	int x;
	int y;

	void p();
	void InitZone();

	virtual void print() override;

	void setPlayerActive(bool active);
	void SetPlayersRegion(int px, int py);
	void ProcessCommand(PlayerCommand c);
};