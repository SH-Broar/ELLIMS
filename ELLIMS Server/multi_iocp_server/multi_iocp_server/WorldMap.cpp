#include "WorldMap.h"

bool s_Map::s_basemap[W_WIDTH][W_HEIGHT];

void s_Map::loadMap()
{
	for (int i = 0; i < W_WIDTH; ++i)
	{
		for (int j = 0; j < W_HEIGHT; ++j)
		{
			s_basemap[i][j] = false;
			if ((i * 5 + j) % 7 == 0) s_basemap[i][j] = true;
		}
	}
}

bool s_Map::canMove(int x, int y)
{
	return !s_basemap[x][y];
}