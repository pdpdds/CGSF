#include "StdAfx.h"
#include "SFTraining.h"

SFTraining::SFTraining(int Mode)
: SFGameMode(Mode)
{
}

SFTraining::~SFTraining(void)
{
}

BOOL SFTraining::OnEnter( int GameMode )
{
	return TRUE;
}

BOOL SFTraining::Onleave()
{
	return TRUE;
}

BOOL SFTraining::Update(DWORD timerId)
{
	return TRUE;
}

BOOL SFTraining::ProcessUserRequest( SFPlayer* pPlayer, int Msg )
{
	return TRUE;
}

BOOL SFTraining::ProcessUserRequest( SFPlayer* pPlayer, BasePacket* pPacket )
{
	return TRUE;
}