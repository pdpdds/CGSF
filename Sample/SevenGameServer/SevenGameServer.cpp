// SevenGameServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "SevenGameProtocol.h"
#include "SevenGameConstant.h"
#include "SGTraining.h"
#include "SGBattle.h"
#include "SFLogicEntry.h"
#include "SFServiceController.h"
#include "SFUtil.h"
#include "SFBreakPad.h"

SFServiceController g_serviceController;
bool ControlService(int argc, _TCHAR** argv);

static DWORD StartSevenGameService(LPDWORD param)
{
	SFBreakPad exceptionHandler;
	exceptionHandler.Install();

	TCHAR szFilePath[MAX_PATH] = { 0, };
	GetModuleFileName(NULL, szFilePath, MAX_PATH);
	TCHAR* szPath = SFUtil::ExtractPathInfo(szFilePath, SFUtil::PATH_DIR);
	SetCurrentDirectory(szPath);

	SFLogicEntry* pLogicEntry = new SFLogicEntry();
	pLogicEntry->AddGameMode(GAMEMODE_TRAINING, new SGTraining(GAMEMODE_TRAINING));
	pLogicEntry->AddGameMode(GAMEMODE_BATTLE, new SGBattle(GAMEMODE_BATTLE));

	/////////////////////////////////////////////////////////////////////
	SFEngine::GetInstance()->Intialize(pLogicEntry, new SFPacketProtocol<SevenGameProtocol>);
	SFEngine::GetInstance()->AddTimer(0, 500, 1000);
	SFEngine::GetInstance()->Start();

	google::FlushLogFiles(google::GLOG_INFO);

#ifdef _DEBUG
	getchar();
#else
	WaitForSingleObject(SFServiceController::killServiceEvent, INFINITE);
#endif

	SFEngine::GetInstance()->ShutDown();

	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc == 2)
	{
		ControlService(argc, argv);
		return 0;
	}

#ifdef _DEBUG	
	StartSevenGameService(NULL);
	
#else	
	g_serviceController.ServiceEntry(argv[0], (LPTHREAD_START_ROUTINE)StartSevenGameService	);
#endif		

	return 0;
}

bool ControlService(int argc, _TCHAR** argv)
{
	TCHAR szFilePath[MAX_PATH] = { 0, };
	TCHAR szFileName[MAX_PATH] = { 0, };
	GetModuleFileName(NULL, szFilePath, MAX_PATH);
	SFUtil::GetFileName(szFileName, MAX_PATH);
	TCHAR* szPath = SFUtil::ExtractPathInfo(szFilePath, SFUtil::PATH_DIR);
	SetCurrentDirectory(szPath);

	if (argc == 2)
	{
		if (_tcsicmp(argv[1], L"-i") == 0)
		{
			return g_serviceController.InstallService(szFileName, szFileName, szFilePath);
		}
		else if (_tcsicmp(argv[1], L"-u") == 0)
		{
			return  g_serviceController.DeleteService(szFileName);
		}

		printf("usage : filename -i(-u)\n");
	}

	return false;
}