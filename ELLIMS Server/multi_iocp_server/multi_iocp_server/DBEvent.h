#pragma once
#include "AMBIT.h"
#include "LoginData.h"

struct DB_EVENT {
	int id;
	DB_EVENT_TYPE EVENT;
	LoginData session;
};