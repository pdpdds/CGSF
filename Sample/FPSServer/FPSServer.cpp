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
	
	SFEngine::GetInstance()->Intialize(pLogicEntry);
	
	SFEngine::GetInstance()->AddPacketProtocol(0, new SFPacketProtocol<FPSProtocol>);
	SFEngine::GetInstance()->Listen(0);

	getchar();

	SFEngine::GetInstance()->ShutDown();

	return 0;
}