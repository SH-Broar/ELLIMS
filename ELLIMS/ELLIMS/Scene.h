#pragma once
#include "Turboc.h"

enum class SceneName { TITLE, INGAME};

class Game;
class Zone;
class Player;

class Scene
{
private:
	std::vector<Zone> areas;
	std::unordered_map <int, Zone> drawableObjects;

	SceneName nowScene;

public:
	Scene();

	void changeScene(SceneName sceneName);
	void UpdateScene(Player& p);
	void printScene();
	void clickScene(int mx, int my, bool clicked);
	void GameZoneInvalid();

	void printDebugConsole();
	const char* mapCalc(int px, int py);

	bool updatable;
};

