// 혼자 연구하는 C/C++의 도우미 헤더 파일
// 비주얼 C++ 환경에서 터보 C 스타일의 함수를 정의한다.

#ifndef __TURBOC_HEADER_
#define __TURBOC_HEADER_

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <time.h>
#include <atomic>
#include <functional>
#include <vector>
#include <array>
#include <unordered_map>

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <winsock.h>
#pragma comment (lib, "ws2_32.lib")

#include <windows.h>

enum class PlayerCommand { NONE, UP, DOWN, LEFT, RIGHT, ESCAPE, BACKSPACE };

#include "..\..\ELLIMS Server\multi_iocp_server\multi_iocp_server\protocol.h"

#include "Network.h"



#define SCREEN_WIDTH 120
#define SCREEN_HEIGHT 35

enum class CURSOR_TYPE { NOCURSOR, SOLIDCURSOR, NORMALCURSOR } ;


void clrscr();
void gotoxy(int x, int y);
int wherex();
int wherey();
void setcursortype(CURSOR_TYPE c);
void CClear();

#define randomize() srand((unsigned)time(NULL))		// 난수 발생기 초기화
#define random(n) (rand() % (n))					//0~n까지의 난수 발생


#endif // TURBOC_HEADER