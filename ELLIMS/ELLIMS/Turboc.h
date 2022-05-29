// ȥ�� �����ϴ� C/C++�� ����� ��� ����
// ���־� C++ ȯ�濡�� �ͺ� C ��Ÿ���� �Լ��� �����Ѵ�.

#ifndef __TURBOC_HEADER_
#define __TURBOC_HEADER_

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <time.h>
#include <windows.h>

#define SCREEN_WIDTH 120
#define SCREEN_HEIGHT 35

enum class CURSOR_TYPE { NOCURSOR, SOLIDCURSOR, NORMALCURSOR } ;
void clrscr();
void gotoxy(int x, int y);
int wherex();
int wherey();
void setcursortype(CURSOR_TYPE c);
void CClear();

#define randomize() srand((unsigned)time(NULL))		// ���� �߻��� �ʱ�ȭ
#define random(n) (rand() % (n))					//0~n������ ���� �߻�

#endif // TURBOC_HEADER
