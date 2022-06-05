#include "Player.h"
#include "Game.h"
#include "Zone.h"


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
	characterIcon = 'O';
	isActive = true;
}

void Player::InitZone()
{
	l = SCREEN_WIDTH / 2;
	r = l;
	t = SCREEN_HEIGHT / 2;
	b = t;

	strcpy(text, &characterIcon);

}

void Player::print()
{
	if (!isActive)
		return;

	Game::DoubleFrameBuffer[l][b] = characterIcon;
}

void Player::SetPlayersRegion(int px, int py)
{
	l = SCREEN_WIDTH / 2 - (x - px);
	r = l;
	t = SCREEN_HEIGHT / 2 - (y - py);
	b = t;

	if (l <= 0 || l >= SCREEN_WIDTH - 1 || b <= 0 || b >= SCREEN_HEIGHT - 1)
		isActive = false;
}

void Player::setPlayerActive(bool active)
{
	isActive = active;
}

void Player::ProcessCommand(PlayerCommand c)
{

}

