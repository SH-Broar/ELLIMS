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
			Game::setFocusZone(areas[3],ClearType::TEXT);
			return 0;
		};

		areas[4] = [&](int mx, int my) -> int {
			Game::setFocusZone(areas[4], ClearType::TEXT);
			return 0;
		};

		areas[5] = [&](int mx, int my) -> int {
			Game::print("ID / Password Invaild!", SCREEN_WIDTH / 2 - 10, 26);
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
	for (auto& z : areas)
	{
		z.setActiveByFrame();
	}
}
