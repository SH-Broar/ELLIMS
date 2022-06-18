#pragma once
#include "AMBIT.h"

class s_Map {
public:
	struct NODE
	{
		int x, y;
		float g;
		float h;
		float f;
		NODE* parent;

		bool operator<(NODE l) const
		{
			return f < l.f;
		}
		bool operator==(NODE l)
		{
			if (x == l.x && y == l.y)
				return true;
			else
				return false;
		}
	};

public:
	static bool s_basemap[W_WIDTH][W_HEIGHT];
	static std::atomic<bool> s_movemap[W_WIDTH][W_HEIGHT];
	static NODE nodeMap[W_WIDTH][W_HEIGHT];
	static void loadMap();

	static bool canMove(int x, int y);
};