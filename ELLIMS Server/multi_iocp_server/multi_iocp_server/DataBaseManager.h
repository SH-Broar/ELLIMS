#pragma once

struct LoginData
{
	bool isValidLogin = false;
	int sc_id;
	char id[100];
	char name[100];
	int level;
	int x, y;
	int HP, MaxHP;
	int MP, MaxMP;
	int EXP;
};

class DataBaseManager
{
private:

public:
	DataBaseManager();

	static LoginData getLoginData(char* name, char* password);
};

