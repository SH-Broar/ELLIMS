#include "Scene.h"
#include "Game.h"
#include "Zone.h"
#include "Player.h"
#include "WorldMap.h"

Scene::Scene()
{
	areas.clear();
	updatable = false;
	nowScene = SceneName::TITLE;
}

void Scene::changeScene(SceneName sceneName)
{
	areas.clear();
	Zone all{};
	all.clearZone(ClearType::FULL);

	nowScene = sceneName;

	switch (sceneName)
	{
	case SceneName::TITLE:
	{
		updatable = false;
		areas.emplace_back(SCREEN_WIDTH / 2 - 6, SCREEN_WIDTH / 2 + 13, 10, 10, ClickableType::NONE);
		areas.emplace_back(SCREEN_WIDTH / 2 - 2, SCREEN_WIDTH / 2 + 10, 19, 19, ClickableType::HOVER);
		areas.emplace_back(SCREEN_WIDTH / 2 - 2, SCREEN_WIDTH / 2 + 10, 22, 22, ClickableType::HOVER);

		areas.emplace_back(SCREEN_WIDTH / 2 - 5, SCREEN_WIDTH / 2 + 5, 19, 19, ClickableType::BUTTON);
		areas.emplace_back(SCREEN_WIDTH / 2 - 5, SCREEN_WIDTH / 2 + 5, 23, 23, ClickableType::BUTTON);
		areas.emplace_back(SCREEN_WIDTH / 2 - 2, SCREEN_WIDTH / 2 + 10, 28, 28, ClickableType::HOVER);

		areas[0] = " E L L I M S ";
		areas[1] = "Start";
		areas[2] = "Close";

		areas[3].setType(FramePrintType::FULL);
		areas[3] = "    ID     ";
		areas[3].setActive(false);
		areas[4].setType(FramePrintType::FULL);
		areas[4] = " Password    ";
		areas[4].setActive(false);

		areas[5] = "LogIn";
		areas[5].setActive(false);

		areas[1] = [&](int mx, int my) -> int {
			areas[1].setActive(false);
			areas[2].setActive(false);
			areas[3].setActive(true);
			areas[4].setActive(true);
			areas[5].setActive(true);
			return 0;
		};

		areas[2] = [](int mx, int my) -> int {
			Game::gameEnded();
			return 0;
		};

		areas[3] = [&](int mx, int my) -> int {
			Game::setFocusZone(areas[3], ClearType::TEXT);
			return 0;
		};

		areas[4] = [&](int mx, int my) -> int {
			Game::setFocusZone(areas[4], ClearType::TEXT);
			return 0;
		};

		areas[5] = [&](int mx, int my) -> int {
			Game::print("Connecting To Server....", SCREEN_WIDTH / 2 - 11, 26);
			Network::NetworkCodex();
			while (!Game::networkConnected) { SleepEx(30, TRUE); }
			changeScene(SceneName::INGAME);
			
			return 0;
		};

	}
	break;
	case SceneName::INGAME:
	{
		updatable = true;
		areas.emplace_back(1, (SCREEN_HEIGHT-2)*2, 1, SCREEN_HEIGHT-2);
		areas[0].setType(FramePrintType::FULL);
	}
		break;
	default:
		break;
	}
}

void Scene::UpdateScene(Player& p)
{
	if (updatable)
	{
		switch (nowScene)
		{
		case SceneName::INGAME:
		{
			areas[0] = mapCalc(p.x, p.y);
		}
			break;
		default:
			break;
		}
	}
}

void Scene::printScene()
{
	for (auto& z : areas)
	{
		z.print();
	}
}

void Scene::clickScene(int mx, int my, bool clicked)
{
	for (auto& z : areas)
	{
		z.MouseInteraction(mx, my, clicked);
	}
	for (auto& z : areas)
	{
		z.setActiveByFrame();
	}
}

void Scene::printDebugConsole()
{
#ifdef DEBUG
	Zone all{};
	all.clearZone(ClearType::FULL);
#else
	printScene();
#endif
}

const char* Scene::mapCalc(int px, int py)
{
	char tmp[SCREEN_HEIGHT*2 * SCREEN_HEIGHT] = {};

	for (int i = 0; i < (SCREEN_HEIGHT -2)*2; ++i)
	{
		for (int j = 0; j < SCREEN_HEIGHT-2; ++j)
		{
			int tile_x = i + px - (SCREEN_HEIGHT-2);
			int tile_y = j + py - (SCREEN_HEIGHT / 2-1);

			if (tile_x >= 0 && tile_y >= 0 && tile_x < W_WIDTH && tile_y < W_HEIGHT)
				tmp[i + j * (SCREEN_HEIGHT - 2) * 2] = c_Map::c_basemap[tile_x][tile_y];
			else
				tmp[i + j * (SCREEN_HEIGHT - 2) * 2] = ' ';
		}
	}

	return tmp;
}