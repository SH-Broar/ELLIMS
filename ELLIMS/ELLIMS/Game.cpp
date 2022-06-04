#include "Game.h"

bool Game::gameEnd = false;
std::atomic<bool> Game::newCharInputed;
char Game::inputChar;
Zone* Game::focusedZone = nullptr;

int Game::GameStart()
{
	//���⼭ �ʱ�ȭ
	SleepEx(25, TRUE);

	nowScene.changeScene(SceneName::TITLE);

	while (true)
	{
		//hover and click
		nowScene.clickScene(mouse_X, mouse_Y, mouse_Left_down_Event);

		if (mouse_Left_down_Event)
		{
			/*Zone tmpz{ mouse_X,mouse_X + 9,mouse_Y,mouse_Y, ClickableType::HOVER };
			tmpz = "12345678";
			tmpz.print();*/
		}
		if (newCharInputed)
		{
			if (focusedZone != nullptr)
				focusedZone->addTyping(inputChar);
		}
		//contents
		// �� ���
		nowScene.printScene();
		// ���� ���
		// ä�� ���

		//CClear();
		//std::cout << mouse_X;

		//break;
		if (gameEnd)
			break;
		mouse_Left_down_Event = false;
		newCharInputed = false;
		SleepEx(25, TRUE);
	}
	return 0;
}

void Game::setFocusZone(Zone& z, ClearType c)
{
	z.clearZone(c);
	z = " ";

	focusedZone = &z;
}