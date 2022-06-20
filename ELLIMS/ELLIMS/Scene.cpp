#include "Turboc.h"
#include "Scene.h"
#include "Game.h"
#include "Zone.h"
#include "Player.h"
#include "WorldMap.h"
#include "ChatDialogue.h"

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
		bool newUser = false;
		areas.reserve(6);

		updatable = false;
		areas.emplace_back(SCREEN_WIDTH / 2 - 6, SCREEN_WIDTH / 2 + 13, 10, 10, ClickableType::NONE);
		areas.emplace_back(SCREEN_WIDTH / 2 - 2, SCREEN_WIDTH / 2 + 10, 19, 19, ClickableType::HOVER);
		areas.emplace_back(SCREEN_WIDTH / 2 - 3, SCREEN_WIDTH / 2 + 10, 22, 22, ClickableType::HOVER);

		areas.emplace_back(SCREEN_WIDTH / 2 - 5, SCREEN_WIDTH / 2 + 5, 19, 19, ClickableType::BUTTON);
		areas.emplace_back(SCREEN_WIDTH / 2 - 5, SCREEN_WIDTH / 2 + 5, 23, 23, ClickableType::BUTTON);
		areas.emplace_back(SCREEN_WIDTH / 2 - 3, SCREEN_WIDTH / 2 + 10, 28, 28, ClickableType::HOVER);

		areas[0] = " E L L I M S ";
		areas[1] = "Start";
		areas[2] = "Sign In";

		areas[3].setType(FramePrintType::FULL);
		areas[3] = "    ID     ";
		areas[3].setActive(false);
		areas[4].setType(FramePrintType::FULL);
		areas[4] = " Password    ";
		areas[4].setActive(false);

		areas[5] = "Game In";
		areas[5].setActive(false);

		areas[1] = [&](int mx, int my) -> int {
			areas[1].setActive(false);
			areas[2].setActive(false);
			areas[3].setActive(true);
			areas[4].setActive(true);
			areas[5].setActive(true);
			return 0;
		};

		areas[2] = [&](int mx, int my) -> int {
			newUser = true;
			areas[1].setActive(false);
			areas[2].setActive(false);
			areas[3].setActive(true);
			areas[4].setActive(true);
			areas[5] = "Sign In";
			areas[5].setActive(true);
			return 0;
		};

		areas[3] = [&](int mx, int my) -> int {
			if (!Game::isthisFocused(areas[3]))
				Game::setFocusZone(areas[3], ClearType::TEXT);
			return 0;
		};
		areas[3] = [&](void) -> void {
			if (Game::isthisFocused(areas[3]))
				Game::setFocusZone(nullptr);
			return;
		};

		areas[4] = [&](int mx, int my) -> int {
			if (!Game::isthisFocused(areas[4]))
				Game::setFocusZone(areas[4], ClearType::TEXT);
			return 0;
		};
		areas[4] = [&](void) -> void {
			if (Game::isthisFocused(areas[4]))
				Game::setFocusZone(nullptr);
			return;
		};

		areas[5] = [&](int mx, int my) -> int
		{
			Game::print("                           ", SCREEN_WIDTH / 2 - 11, 26);
			if (!Game::networkConnected)
			{
				Game::print("Connecting To Server....", SCREEN_WIDTH / 2 - 11, 26);
				Network::NetworkCodex(areas[3].getText(), areas[4].getText(), newUser);
				//while (!Game::networkConnected) { SleepEx(30, TRUE); }
				Game::DBConnected = true;
			}
			else
			{
				Game::print("Connecting To Server....", SCREEN_WIDTH / 2 - 11, 26);
				Network::TryLogin(areas[3].getText(), areas[4].getText(), newUser);
				Game::DBConnected = true;
			}

			int cont = 0;
			while (cont < 1000 && Game::DBConnected)
			{
				cont += 1;
				if (Game::ingame)
				{
					changeScene(SceneName::INGAME);
					break;
				}
				SleepEx(1, TRUE);
			}
			//Game::print("                           ", SCREEN_WIDTH / 2 - 11, 26);
			//Game::print("LogIn Failed! ", SCREEN_WIDTH / 2 - 6, 26);
			return 0;
		};

	}
	break;
	case SceneName::INGAME:
	{
		updatable = true;

		constexpr int sideL = (SCREEN_HEIGHT - 2) * 2 + 2;
		constexpr int sideR = SCREEN_WIDTH - 2;
		constexpr int sideLL = (sideL + sideR) / 2 - 1;

		areas.reserve(10);

		//맵
		areas.emplace_back(1, (SCREEN_HEIGHT-2)*2, 1, SCREEN_HEIGHT-2);
		areas[0].setType(FramePrintType::FULL);

		//플레이어 정보
		areas.emplace_back(sideL, sideR, 1, SCREEN_HEIGHT/4-1);
		areas[1].setType(FramePrintType::FULL);
		//뭔가 다른거
		areas.emplace_back(sideL, sideR, SCREEN_HEIGHT/4+1, SCREEN_HEIGHT / 4*2 -2);
		areas[2].setType(FramePrintType::FULL);
		//채팅
		areas.emplace_back(sideL, sideR, SCREEN_HEIGHT / 4 * 2, SCREEN_HEIGHT -4);
		areas[3].setType(FramePrintType::FULL);
		Game::chat.setZone(&areas[3]);
		//채팅 입력존 
		areas.emplace_back(sideL, sideR, SCREEN_HEIGHT -2, SCREEN_HEIGHT-2, ClickableType::BUTTON);
		areas[4].setType(FramePrintType::FULL);
		areas[4] = [&](int mx, int my) -> int {
			Game::setFocusZone(areas[4], ClearType::TEXT);
			return 0;
		};
		areas[4] = [&](void) -> void {
			Network::SendChat(areas[4].getText());
			Game::setFocusZone(areas[4], ClearType::TEXT);
			return;
		};

		//이름 레벨 //직업
		areas.emplace_back(sideL +1, sideLL-1, 1, 1, ClickableType::NONE);
		areas.emplace_back(sideLL + 2, sideR-2, 1, 1, ClickableType::NONE);
		
		//areas.emplace_back(SCREEN_WIDTH / 2 - 6, SCREEN_WIDTH / 2 + 13, 10, 10, ClickableType::NONE);

		//HPMP
		areas.emplace_back(sideL, sideR, 3, 3, ClickableType::NONE);
		areas.emplace_back(sideL, sideR, 4, 4, ClickableType::NONE);

		//경험치
		areas.emplace_back(sideL, sideR, 6, 6, ClickableType::NONE);
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

			char hmp[30]{};

			areas[5] = p.name;

			sprintf(hmp, " Lv. %3d", p.level);
			areas[6] = hmp;
			ZeroMemory(hmp, sizeof(hmp));
			sprintf(hmp, " HP  : %8d / %8d", p.HP, p.MaxHP);
			areas[7] = hmp;
			ZeroMemory(hmp, sizeof(hmp));
			sprintf(hmp, " MP  : %8d / %8d", p.MP, p.MaxMP);
			areas[8] = hmp;
			ZeroMemory(hmp, sizeof(hmp));
			sprintf(hmp, " EXP : %8d / %8d", p.EXP, p.MaxEXP);
			areas[9] = hmp;
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
	bool catched = false;
	for (auto& z : areas)
	{
		catched |= z.MouseInteraction(mx, my, clicked);
	}
	if (clicked && !catched)
		Game::setFocusZone(nullptr);

	for (auto& z : areas)
	{
		z.setActiveByFrame();
	}
}

void Scene::GameZoneInvalid()
{
	for (auto& z : areas)
	{
		z.ZoneChanged = true;
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