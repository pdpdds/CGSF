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

BOOL SFGameModeFSM::OnEnter( int gameMode )
{
	GameModeMap::iterator iter = m_GameModeMap.find(gameMode);

	if(iter == m_GameModeMap.end())
		return FALSE;

	m_pCurrentGameMode = iter->second;

	m_pCurrentGameMode->OnEnter(gameMode);

	return TRUE;
}

BOOL SFGameModeFSM::SetGameMode(int gameMode)
{
	GameModeMap::iterator iter = m_GameModeMap.find(gameMode);

	if(iter == m_GameModeMap.end())
		return FALSE;

	m_pCurrentGameMode = iter->second;

	return TRUE;
}

BOOL SFGameModeFSM::Onleave()
{
	return m_pCurrentGameMode->Onleave();
}

BOOL SFGameModeFSM::Update(DWORD timerId)
{
	if(m_pCurrentGameMode)
		return m_pCurrentGameMode->Update(timerId);

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

BOOL SFGameModeFSM::ProcessUserRequest( SFPlayer* pPlayer, BasePacket* pPacket )
{
	return m_pCurrentGameMode->ProcessUserRequest(pPlayer, pPacket);
}

BOOL SFGameModeFSM::ProcessUserRequest( SFPlayer* pPlayer, int Msg )
{
	return m_pCurrentGameMode->ProcessUserRequest(pPlayer, Msg);
}