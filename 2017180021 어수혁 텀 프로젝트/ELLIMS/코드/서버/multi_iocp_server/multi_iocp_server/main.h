#pragma once
#include "AMBIT.h"

class SESSION;
class DataBaseManager;
class HeartManager;

extern array<SESSION, MAX_USER+ NUM_NPC> clients;
extern HANDLE g_h_iocp;
extern SOCKET g_s_socket;

extern concurrent_unordered_map<int, concurrent_unordered_map<int, concurrent_unordered_map<int, bool>>> sectors;
