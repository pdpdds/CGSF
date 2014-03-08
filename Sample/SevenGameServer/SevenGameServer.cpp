// SevenGameServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "SFEngine.h"
#include "GameModeConstant.h"
#include "SFLogicEntry.h"
#include "SevenGameProtocol.h"

int _tmain(int argc, _TCHAR* argv[])
{
	SFLogicEntry* pLogicEntry = new SFLogicEntry();
	pLogicEntry->AddGameMode(GAMEMODE_TRAINING, new SGTraining(GAMEMODE_TRAINING));
	pLogicEntry->AddGameMode(GAMEMODE_BATTLE, new SGBattle(GAMEMODE_BATTLE));
	
/////////////////////////////////////////////////////////////////////
	SFASSERT(TRUE == SFEngine::GetInstance()->Intialize(pLogicEntry, new SFPacketProtocol<SevenGameProtocol>));
	SFASSERT(TRUE == SFEngine::GetInstance()->AddTimer(0, 500, 1000));
	SFASSERT(TRUE == SFEngine::GetInstance()->Start());
		
	google::FlushLogFiles(google::GLOG_INFO);

	getchar();

	SFEngine::GetInstance()->ShutDown();

	return 0;
}