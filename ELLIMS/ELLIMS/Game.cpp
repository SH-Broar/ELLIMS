#include "Game.h"

bool Game::gameEnd = false;


int Game::GameStart()
{
	//���⼭ �ʱ�ȭ

	while (true)
	{
		if (mouse_Left_down_Event)
		{
			//���⼭ Scene�� ��� zone�� clicked ����
		}

		if (mouse_Left_down_Event)
		{
			Zone tmpz{ mouse_X,mouse_X + 9,mouse_Y,mouse_Y + 1,true };
			tmpz = "1234567890123456789012345678901234567890";
			tmpz.print();
		}
		//contents
		// �� ���
		// ���� ���
		// ä�� ���

		//CClear();
		//std::cout << mouse_X;

		//break;
		SleepEx(25, TRUE);
	}

	gameEnd = true;
	return 0;
}