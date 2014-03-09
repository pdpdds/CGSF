// SevenGameServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "SevenGameProtocol.h"
#include "GameModeConstant.h"
#include "SFLogicEntry.h"
#include "SFServiceController.h"
#include "SFUtil.h"

static DWORD ServiceExecutionThread(LPDWORD param)
{
	SFLogicEntry* pLogicEntry = new SFLogicEntry();
	pLogicEntry->AddGameMode(GAMEMODE_TRAINING, new SGTraining(GAMEMODE_TRAINING));
	pLogicEntry->AddGameMode(GAMEMODE_BATTLE, new SGBattle(GAMEMODE_BATTLE));

	/////////////////////////////////////////////////////////////////////
	SFASSERT(TRUE == SFEngine::GetInstance()->Intialize(pLogicEntry, new SFPacketProtocol<SevenGameProtocol>));
	SFASSERT(TRUE == SFEngine::GetInstance()->AddTimer(0, 500, 1000));
	SFASSERT(TRUE == SFEngine::GetInstance()->Start());

	google::FlushLogFiles(google::GLOG_INFO);

	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	SFServiceController Controller;
	TCHAR szFilePath[MAX_PATH] = { 0, };
	TCHAR szFileName[MAX_PATH] = { 0, };
	GetModuleFileName(NULL, szFilePath, MAX_PATH);
	SFUtil::GetProgramName(szFileName, MAX_PATH);

	if (argc == 2)
	{				
		

		if (_tcsicmp(argv[1], L"-i") == 0)
		{
			return Controller.InstallService(szFileName, szFileName, szFilePath);
		}
		else if (_tcsicmp(argv[1], L"-u") == 0)
		{
			return  Controller.DeleteService(szFileName);
		}

		printf("usage : filename -i(-u)\n");

		return 0;
	}

#ifdef _DEBUG
	{
		ServiceExecutionThread(NULL);
	}
#else
	{
		
		Controller.ServiceEntry(szFileName, (LPTHREAD_START_ROUTINE)ServiceExecutionThread);
	}
#endif

	getchar();

	SFEngine::GetInstance()->ShutDown();

	return 0;
}