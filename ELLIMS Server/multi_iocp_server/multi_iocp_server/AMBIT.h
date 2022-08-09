#pragma once

#ifndef DEFINE_AMBIT
#define DEFINE_AMBIT

#include <iostream>
#include <array>
//#include <concurrent_unordered_map.h>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <atomic>
#include <vector>
#include <queue>
#include <chrono>
#include <thread>
#include <mutex>
#include <memory>
#include <unordered_set>
#include <concurrent_unordered_map.h>
#include <concurrent_queue.h>
#include <concurrent_priority_queue.h>

extern "C"
{
#include "include/lua.h"
#include "include/lauxlib.h"
#include "include/lualib.h"
}

#pragma comment(lib, "lua54.lib")
#pragma comment(lib,"WS2_32.lib")
#pragma comment(lib,"MSWSock.lib")

using namespace std;
using namespace concurrency;
enum COMP_TYPE { OP_ACCEPT, OP_RECV, OP_SEND, OP_DB, OP_AI };
enum SESSION_STATE { ST_DUMMY, ST_NPC_SLEEP, ST_NPC_DEAD, ST_FREE, ST_ACCEPTED, ST_INGAME };
enum TIMER_EVENT_TYPE { EV_DUMMY, EV_MOVE, EV_HEAL, EV_ATTACK, EV_RESURRECTION };
enum DB_EVENT_TYPE { DB_EV_LOGIN, DB_EV_LOGOUT, DB_EV_NEWUSER };

#include "protocol.h"
#include "SESSION.h"
#include "DataBaseManager.h"
#include "HeartManager.h"
#include "WorldMap.h"


constexpr int RANGE = 15;
constexpr int SECTOR_WIDTH = 20;
constexpr int SECTOR_HEIGHT = 20;

int distance_cell(int a, int b);



#endif // !DEFINE_AMBIT

