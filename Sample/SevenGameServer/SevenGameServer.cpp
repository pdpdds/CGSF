// SevenGameServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "SFEngine.h"
#include "GameModeConstant.h"
#include "SFLogicEntry.h"
#include "SevenGameProtocol.h"
#include "SFCasualGameDispatcher.h"

SFEngine* g_pEngine = NULL;

int _tmain(int argc, _TCHAR* argv[])
{
	g_pEngine = new SFEngine(argv[0]);

	SFLogicEntry* pLogicEntry = new SFLogicEntry();
	pLogicEntry->AddGameMode(GAMEMODE_TRAINING, new SGTraining(GAMEMODE_TRAINING));
	pLogicEntry->AddGameMode(GAMEMODE_BATTLE, new SGBattle(GAMEMODE_BATTLE));
	
/////////////////////////////////////////////////////////////////////
	SFASSERT(TRUE == g_pEngine->Intialize(pLogicEntry, new SFPacketProtocol<SevenGameProtocol>, new SFCasualGameDispatcher()));
	SFASSERT(TRUE == g_pEngine->AddTimer(0, 500, 1000));
	SFASSERT(TRUE == g_pEngine->Start());
		
	google::FlushLogFiles(google::GLOG_INFO);

	getchar();
	g_pEngine->ShutDown();

	return 0;
}