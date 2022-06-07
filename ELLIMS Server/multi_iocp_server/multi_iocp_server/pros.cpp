#pragma once
#include "AMBIT.h"

array<SESSION, MAX_USER> clients;
HANDLE g_h_iocp;
SOCKET g_s_socket;

concurrent_unordered_map<int, concurrent_unordered_map<int, concurrent_unordered_map<int, bool>>> sectors;