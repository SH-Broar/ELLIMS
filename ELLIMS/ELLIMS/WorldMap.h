#pragma once
#include "Turboc.h"

class c_Map {
public:
	static char c_basemap[W_WIDTH][W_HEIGHT];
	static void loadMap();
};

class s_Map {
public:
	static bool s_basemap[W_WIDTH][W_HEIGHT];
	static void loadMap();
};

