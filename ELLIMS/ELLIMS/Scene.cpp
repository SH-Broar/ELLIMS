#include "Scene.h"
#include "Game.h"
Scene::Scene()
{
	areas.clear();
}

void Scene::changeScene(SceneName sceneName)
{
	areas.clear();
	switch (sceneName)
	{
	case SceneName::TITLE:
	{
		areas.emplace_back(SCREEN_WIDTH/2 - 6, SCREEN_WIDTH/2 + 10, 10, 10, ClickableType::NONE);
		areas.emplace_back(SCREEN_WIDTH/2 - 2, SCREEN_WIDTH/2 + 10, 19, 19, ClickableType::HOVER);
		areas.emplace_back(SCREEN_WIDTH/2 - 2, SCREEN_WIDTH/2 + 10, 22, 22, ClickableType::HOVER);

		areas[0] = " E L L I M S ";
		areas[1] = "Start";
		areas[2] = "Close";

		areas[1] = [](int mx, int my) -> int {
			//printf("Commons");
			return 0;
		};

		areas[2] = [](int mx, int my) -> int {
			Game::gameEnded();
			return 0;
		};

	}
		break;
	case SceneName::INGAME:
		break;
	default:
		break;
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
}
