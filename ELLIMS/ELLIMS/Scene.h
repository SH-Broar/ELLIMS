#pragma once
#include <vector>
#include "Zone.h"

enum class SceneName { TITLE, INGAME};

class Scene
{
private:
	std::vector<Zone> areas;

public:
	Scene();

	void changeScene(SceneName sceneName);
	void printScene();
	void clickScene(int mx, int my, bool clicked);

	void printDebugConsole();
};

