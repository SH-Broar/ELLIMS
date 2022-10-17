#pragma once
#include "AMBIT.h"
#include "DBEvent.h"
#include "LoginData.h"

class SESSION;

class DataBaseManager
{
public:


	DataBaseManager();

	static void DBThread();
	static concurrent_queue<DB_EVENT> dbWorkQueue;
	static void addDBEvent(int id, DB_EVENT_TYPE type, LoginData& sess);

	static LoginData getLoginData(char* name, char* password);
	static LoginData newLoginData(char* name, char* password);
	static bool setLoginData(LoginData& data);
};

