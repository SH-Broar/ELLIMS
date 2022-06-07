#pragma once

#ifndef DEFINE_AMBIT
#define DEFINE_AMBIT

#include <iostream>
#include <array>
//#include <concurrent_unordered_map.h>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <vector>
#include <thread>
#include <mutex>
#include <unordered_set>
#include <concurrent_unordered_map.h>

#pragma comment(lib,"WS2_32.lib")
#pragma comment(lib,"MSWSock.lib")

using namespace std;
using namespace concurrency;
enum COMP_TYPE { OP_ACCEPT, OP_RECV, OP_SEND };
enum SESSION_STATE { ST_FREE, ST_ACCEPTED, ST_INGAME };

#include "protocol.h"
#include "SESSION.h"
#include "WorldMap.h"

constexpr int RANGE = 33;
constexpr int SECTOR_WIDTH = 20;
constexpr int SECTOR_HEIGHT = 20;

#endif // !DEFINE_AMBIT

