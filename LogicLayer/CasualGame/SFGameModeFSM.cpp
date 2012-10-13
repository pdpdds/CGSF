#include "StdAfx.h"
#include "SFGameModeFSM.h"


SFGameModeFSM::SFGameModeFSM(int Mode)
: SFGameMode(Mode)
, m_pCurrentGameMode(NULL)
{
}

SFGameModeFSM::~SFGameModeFSM(void)
{
}

BOOL SFGameModeFSM::Initialize(SFRoomPlay* pOwner)
{	
	AddGameMode(pOwner);

	return TRUE;
}

BOOL SFGameModeFSM::OnEnter( int GameMode )
{
	GameModeMap::iterator iter = m_GameModeMap.find(GameMode);

	if(iter == m_GameModeMap.end())
		return FALSE;

	m_pCurrentGameMode = iter->second;

	m_pCurrentGameMode->OnEnter(GameMode);

	return TRUE;
}

BOOL SFGameModeFSM::Onleave()
{
	return m_pCurrentGameMode->Onleave();
}

BOOL SFGameModeFSM::Update( DWORD dwTickcount )
{
	if(m_pCurrentGameMode)
		return m_pCurrentGameMode->Update(dwTickcount);

	return FALSE;
}

BOOL SFGameModeFSM::AddGameMode(SFRoomPlay* pOwner)
{
	SFLogicEntry::GameModeMap* pMap = SFLogicEntry::GetLogicEntry()->GetGameModeMap();

	for(SFLogicEntry::GameModeMap::iterator iter = pMap->begin(); iter != pMap->end(); iter++)
	{
		SFGameMode* pMode = iter->second;
		SFGameMode* pCloneMode = pMode->Clone();
		pCloneMode->SetOwner(pOwner);

		m_GameModeMap.insert(GameModeMap::value_type(iter->first, pCloneMode));
	}

	return TRUE;
}

BOOL SFGameModeFSM::ProcessUserRequest( SFPlayer* pPlayer, SFPacket* pPacket )
{
	return m_pCurrentGameMode->ProcessUserRequest(pPlayer, pPacket);
}

BOOL SFGameModeFSM::ProcessUserRequest( SFPlayer* pPlayer, int Msg )
{
	return m_pCurrentGameMode->ProcessUserRequest(pPlayer, Msg);
}