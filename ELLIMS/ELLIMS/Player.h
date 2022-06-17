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

	bool thisIsPlayer;
	bool deleteDataTicket;

	void p();
	void m(bool boss);
	void InitZone();

	virtual bool print() override;

	void setPlayerActive(bool active);
	bool SetPlayersRegion(int px, int py);
	void ProcessCommand(PlayerCommand c);
};