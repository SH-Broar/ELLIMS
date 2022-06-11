#pragma once

#ifndef DEFINE_AMBIT
#define DEFINE_AMBIT

#include <iostream>
#include <array>
//#include <concurrent_unordered_map.h>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <unordered_set>
#include <concurrent_unordered_map.h>
#include <concurrent_queue.h>

#pragma comment(lib,"WS2_32.lib")
#pragma comment(lib,"MSWSock.lib")

using namespace std;
using namespace concurrency;
enum COMP_TYPE { OP_ACCEPT, OP_RECV, OP_SEND, OP_DB, OP_AI };
enum SESSION_STATE { ST_FREE, ST_ACCEPTED, ST_INGAME };
enum TIMER_EVENT_TYPE { EV_MOVE, EV_HEAL, EV_ATTACK };
enum DB_EVENT_TYPE { DB_EV_LOGIN, DB_EV_LOGOUT, DB_EV_DUMMY };

#include "protocol.h"
#include "SESSION.h"
#include "DataBaseManager.h"
#include "HeartManager.h"
#include "WorldMap.h"


constexpr int RANGE = 33;
constexpr int SECTOR_WIDTH = 20;
constexpr int SECTOR_HEIGHT = 20;



#endif // !DEFINE_AMBIT

