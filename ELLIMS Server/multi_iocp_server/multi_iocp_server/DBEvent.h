#pragma once
#include "AMBIT.h"
#include "LoginData.h"

struct DB_EVENT {
	int id;
	bool isNewUser;
	DB_EVENT_TYPE EVENT;
	LoginData session;
};