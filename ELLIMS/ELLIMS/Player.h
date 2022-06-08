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
	int scID;
	char characterIcon;
	char name[20];

	int x;
	int y;

	int level;
	int MaxHP;
	int HP;
	int MaxMP;
	int MP;

	int MaxEXP;
	int EXP;

	void p();
	void InitZone();

	virtual void print() override;

	void setPlayerActive(bool active);
	void SetPlayersRegion(int px, int py);
	void ProcessCommand(PlayerCommand c);
};