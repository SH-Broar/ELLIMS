#include "Game.h"

bool Game::gameEnd = false;

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
		SleepEx(25, TRUE);
	}
	return 0;
}