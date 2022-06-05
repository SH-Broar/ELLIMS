#include "Player.h"
#include "Game.h"
#include "Zone.h"
#include "Network.h"


Player::Player()
{
	isActive = false;
	characterIcon = 'X';

	pID = 0;
	x = 0;
	y = 0;

	InitZone();
}


void Player::p()
{
	characterIcon = 'T';
	isActive = true;
}

void Player::InitZone()
{
	l = SCREEN_WIDTH / 2;
	r = l;
	t = SCREEN_HEIGHT / 2;
	b = t;

	strcpy(text, &characterIcon);
	isClickable = ClickableType::HOVER;
}

void Player::print()
{
	if (!isActive)
		return;

	if (l <= 0 || l >= SCREEN_WIDTH - 1 || b <= 0 || b >= SCREEN_HEIGHT - 1)
		return;

	Game::DoubleFrameBuffer[l][b] = characterIcon;

	if (hovered)
	{
		if (b > 0)
		{
			for (int i = 0; i < strlen(name); i++)
			{
				Game::DoubleFrameBuffer[l - (strlen(name) / 2) + i][b - 1] = name[i];
			}
		}
	}
}

void Player::SetPlayersRegion(int px, int py)
{
	l = (SCREEN_HEIGHT / 2)*2 - (px - x);
	r = l;
	t = SCREEN_HEIGHT / 2 - (py - y);
	b = t;
}

void Player::setPlayerActive(bool active)
{
	isActive = active;
}

void Player::ProcessCommand(PlayerCommand c)
{
	switch (c)
	{
	case PlayerCommand::UP:
		Network::SendMove(0);
		break;
	case PlayerCommand::DOWN:
		Network::SendMove(1);
		break;
	case PlayerCommand::LEFT:
		Network::SendMove(2);
		break;
	case PlayerCommand::RIGHT:
		Network::SendMove(3);
		break;
	}
}

