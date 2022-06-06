#include "WorldMap.h"

char c_Map::c_basemap[W_WIDTH][W_HEIGHT];
bool s_Map::s_basemap[W_WIDTH][W_HEIGHT];

void c_Map::loadMap()
{
	for (int i = 0; i < W_WIDTH;++i)
	{
		for (int j = 0; j < W_HEIGHT; ++j)
		{
			c_basemap[i][j] = ' ';
			if ((i*5 + j) % 7 == 0) c_basemap[i][j] = '.';
			if ((i == 0 && j > 0) || (j == 0 && i>0)) c_basemap[i][j] = '|';
			if ((i == W_WIDTH-1 && j < W_HEIGHT-1) || (j == W_HEIGHT-1 && i < W_WIDTH-1)) c_basemap[i][j] = '|';
		}
	}
}

void s_Map::loadMap()
{
	for (int i = 0; i < W_WIDTH; ++i)
	{
		for (int j = 0; j < W_HEIGHT; ++j)
		{
			s_basemap[i][j];
		}
	}
}
