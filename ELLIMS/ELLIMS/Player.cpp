#include "Turboc.h"
#include "Player.h"
#include "Game.h"
#include "Zone.h"
#include "Network.h"




Player::Player()
{
	isActive = false;
	characterIcon = 'T';

	scID = 0;
	x = 0;
	y = 0;

	level = 1;

	MaxHP = 50;
	HP = 50;
	MaxMP = 10;
	MP = 10;

	MaxEXP = 100;
	EXP = 0;

	thisIsPlayer = false;
	ZoneChanged = true;
	deleteDataTicket = false;

	InitZone();
}


void Player::p()
{
	characterIcon = 'P';
	isActive = true;
	thisIsPlayer = true;
}

void Player::m()
{
	switch (race)
	{
		//player
	case 0:
		characterIcon = 'T';
		thisIsPlayer = true;
		break;

		//monster
	case 1:
		characterIcon = 'x';
		break;
	case 2:
		characterIcon = 'o';
		break;
	case 3:
		characterIcon = 'z';
		break;
	}

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

bool Player::print()
{
	if (!isActive)
		return false;

	if (l <= 0 || l >= SCREEN_WIDTH - 1 || b <= 0 || b >= SCREEN_HEIGHT - 1)
		return false;

	Game::DoubleFrameBuffer[l][b] = characterIcon;

	if (!thisIsPlayer)
	{
		static std::string hpstatus;
		int len = hpstatus.length();
		hpstatus = std::to_string(HP);
		hpstatus += " / ";
		hpstatus += std::to_string(MaxHP);
		if (len != hpstatus.length())
			zoneChanged();
		for (int i = 0; i < hpstatus.length(); i++)
		{
			Game::DoubleFrameBuffer[l - hpstatus.length() / 2 + i][b + 1] = hpstatus[i];
		}
	}


	if (hovered)
	{
		if (b > 0)
		{
			for (int i = 0; i < strlen(name); i++)
			{
				Game::DoubleFrameBuffer[l - (strlen(name) / 2) + i][b - 1] = name[i];
			}
		}
		zoneChanged();
		deleteDataTicket = false;
	}
	else
	{
		if (!deleteDataTicket)
		{
			zoneChanged();
			deleteDataTicket = true;
			return true;
		}
	}
	return false;
}

bool Player::SetPlayersRegion(int px, int py)
{
	bool ret = false;
	if (l != (SCREEN_HEIGHT / 2) * 2 - (px - x))
	{
		ZoneChanged = true;
		ret = true;
	}
	l = (SCREEN_HEIGHT / 2)*2 - (px - x);
	r = l;
	if (t != SCREEN_HEIGHT / 2 - (py - y))
	{
		ZoneChanged = true;
		ret = true;
	}
	t = SCREEN_HEIGHT / 2 - (py - y);
	b = t;

	return ret;
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

void Player::ProcessCommand(char c)
{
	switch (c)
	{
		//일반 공격
	case 'z':
		Network::SendAttack(0);
		//이펙트 처리
		break;
	case 'x':
		break;
	case 'c':
		break;
	}
}

