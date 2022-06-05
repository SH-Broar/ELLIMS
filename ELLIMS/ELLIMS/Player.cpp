#include "Player.h"

Player::Player()
{
	isActive = false;
	characterIcon = 'X';

	x=0;
	y=0;
}

Player::Player(bool isThisPlayer)
{
	isActive = true;
	characterIcon = 'O';

	x = 0;
	y = 0;
}
