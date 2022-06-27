#pragma once

struct LoginData
{
public:
	bool isValidLogin = false;
	int sc_id;
	char id[100];
	char pass[100];
	char name[100];
	bool isPlayer;
	int level;
	int x, y;
	int HP, MaxHP;
	int MP, MaxMP;
	int EXP;
	int race;
};