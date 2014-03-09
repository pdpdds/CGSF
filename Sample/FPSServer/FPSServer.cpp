#include "stdafx.h"
#include "SFEngine.h"
#include "GameModeConstant.h"
#include "SFLogicEntry.h"
#include "FPSProtocol.h"

int _tmain(int argc, _TCHAR* argv[])
{
	

	SFLogicEntry* pLogicEntry = new SFLogicEntry();
	pLogicEntry->AddGameMode(GAMEMODE_TRAINING, new SFTraining(GAMEMODE_TRAINING));
	pLogicEntry->AddGameMode(GAMEMODE_FREEFORALL, new SFFreeForAll(GAMEMODE_FREEFORALL));
	
	/////////////////////////////////////////////////////////////////////
	SFASSERT(TRUE == SFEngine::GetInstance()->Intialize(pLogicEntry, new SFPacketProtocol<FPSProtocol>));	
	SFASSERT(TRUE == SFEngine::GetInstance()->Start());

	google::FlushLogFiles(google::GLOG_INFO);

	getchar();

	SFEngine::GetInstance()->ShutDown();

	return 0;
}