#include "StdAfx.h"
#include "SFTeam.h"
#include "SFPlayer.h"

SFTeam::SFTeam(void)
{
}

SFTeam::~SFTeam(void)
{
}

BOOL SFTeam::AddMember( SFPlayer* pPlayer )
{
	AddObserver(pPlayer);
	m_MemberMap.insert(make_pair(pPlayer->GetSerial(), pPlayer));

	return TRUE;
}

BOOL SFTeam::DelMember( SFPlayer* pPlayer )
{
	DelObserver(pPlayer);
	m_MemberMap.erase(pPlayer->GetSerial());

	return TRUE;
}