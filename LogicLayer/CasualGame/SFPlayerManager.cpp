#include "stdafx.h"
#include "SFPlayerManager.h"
#include "SFPlayer.h"

SFPlayerManager::SFPlayerManager(void)
{
}


SFPlayerManager::~SFPlayerManager(void)
{
	
}

bool SFPlayerManager::AddPlayer(SFPlayer* pPlayer)
{
	std::pair<std::map<std::string, SFPlayer*>::iterator,bool> ret;
	ret =  m_PlayerMap.insert(std::make_pair(pPlayer->GetPlayerName(), pPlayer));
		
	if(ret.second == false)
		return false;

	return true;
}

void SFPlayerManager::DelPlayer(SFPlayer* pPlayer)
{
	m_PlayerMap.erase(pPlayer->GetPlayerName());
}
