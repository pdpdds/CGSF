#include "stdafx.h"
#include "GameUser.h"


GameUser::GameUser()
	: m_serial(-1)
{
}


GameUser::~GameUser()
{
}

void GameUser::SetName(int serial)
{
	m_szName = "test";
	m_szName += std::to_string(serial);
}


