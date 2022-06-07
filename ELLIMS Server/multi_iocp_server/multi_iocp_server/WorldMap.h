#pragma once
#include "AMBIT.h"

class s_Map {

public:
	static bool s_basemap[W_WIDTH][W_HEIGHT];
	static void loadMap();

	static bool canMove(int x, int y);
};