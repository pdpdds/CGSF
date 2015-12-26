#include "StdAfx.h"
#include "GameUser.h"

GameUser::GameUser(void)
: m_serial(-1)
{
}

GameUser::~GameUser(void)
{
}

void GameUser::SetName(int serial)
{
	m_szName = "test";
	m_szName += std::to_string(serial);
}
