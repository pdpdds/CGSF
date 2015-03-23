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

SERVICE_STATUS        g_ServiceStatus = { 0 };
SERVICE_STATUS_HANDLE g_StatusHandle = NULL;
HANDLE                g_ServiceStopEvent = INVALID_HANDLE_VALUE;

bool ControlService(int argc, _TCHAR** argv);
VOID WINAPI ServiceMain(DWORD argc, LPTSTR *argv);
VOID WINAPI ServiceCtrlHandler(DWORD);

//서비스를 구현한 함수
DWORD WINAPI ServiceWorkerThread(LPVOID lpParam);

int _tmain(int argc, _TCHAR* argv[])
{
//예외 핸들러 등록
	SFBreakPad exceptionHandler;
	exceptionHandler.Install();

//파라메터가 존재할 경우 서비스 등록 및 삭제 루틴을 실행한다.	
	if (argc == 2)
	{
		ControlService(argc, argv);
		return 0;
	}

#ifdef _DEBUG
	ServiceWorkerThread(NULL);
#else
	//서비스 메인 등록
	SERVICE_TABLE_ENTRY ServiceTable[] =
	{
		{ argv[0], (LPSERVICE_MAIN_FUNCTION)ServiceMain },
		{ NULL, NULL }
	};

	if (StartServiceCtrlDispatcher(ServiceTable) == FALSE)
	{
		return 0;
	}
#endif

	return 0;
}

DWORD WINAPI ServiceWorkerThread(LPVOID lpParam)
{
	TCHAR szFilePath[MAX_PATH] = { 0, };
	GetModuleFileName(NULL, szFilePath, MAX_PATH);
	TCHAR* szPath = SFUtil::ExtractPathInfo(szFilePath, SFUtil::PATH_DIR);
	SetCurrentDirectory(szPath);

	SFLogicEntry* pLogicEntry = new SFLogicEntry();
	pLogicEntry->AddGameMode(GAMEMODE_TRAINING, new SGTraining(GAMEMODE_TRAINING));
	pLogicEntry->AddGameMode(GAMEMODE_BATTLE, new SGBattle(GAMEMODE_BATTLE));

	/////////////////////////////////////////////////////////////////////
	SFEngine::GetInstance()->Intialize(pLogicEntry);
	SFEngine::GetInstance()->AddPacketProtocol(0, new SFPacketProtocol<SevenGameProtocol>);
	SFEngine::GetInstance()->AddTimer(0, 500, 1000);
	SFEngine::GetInstance()->Start(0);

	google::FlushLogFiles(google::GLOG_INFO);

#ifdef _DEBUG
	getchar();
#else
	while (WaitForSingleObject(g_ServiceStopEvent, 0) != WAIT_OBJECT_0)
	{
		Sleep(3000);
	}
#endif

	SFEngine::GetInstance()->ShutDown();

	return ERROR_SUCCESS;
}

bool ControlService(int argc, _TCHAR** argv)
{
	SFServiceController serviceController;

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
			return serviceController.InstallService(szFileName, szFileName, szFilePath);
		}
		else if (_tcsicmp(argv[1], L"-u") == 0)
		{
			return  serviceController.DeleteService(szFileName);
		}

		printf("usage : filename -i(-u)\n");
	}

	return false;
}

VOID WINAPI ServiceMain(DWORD argc, LPTSTR *argv)
{
	DWORD Status = E_FAIL;

	g_StatusHandle = RegisterServiceCtrlHandler(argv[0], ServiceCtrlHandler);

	if (g_StatusHandle == NULL)
	{
		goto EXIT;
	}

	// Tell the service controller we are starting
	ZeroMemory(&g_ServiceStatus, sizeof (g_ServiceStatus));
	g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	g_ServiceStatus.dwControlsAccepted = 0;
	g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
	g_ServiceStatus.dwWin32ExitCode = 0;
	g_ServiceStatus.dwServiceSpecificExitCode = 0;
	g_ServiceStatus.dwCheckPoint = 0;

	SetServiceStatus(g_StatusHandle, &g_ServiceStatus);

	// Create stop event to wait on later.
	g_ServiceStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (g_ServiceStopEvent == NULL)
	{
		g_ServiceStatus.dwControlsAccepted = 0;
		g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		g_ServiceStatus.dwWin32ExitCode = GetLastError();
		g_ServiceStatus.dwCheckPoint = 1;

		SetServiceStatus(g_StatusHandle, &g_ServiceStatus);

		goto EXIT;
	}

	// Tell the service controller we are started
	g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
	g_ServiceStatus.dwWin32ExitCode = 0;
	g_ServiceStatus.dwCheckPoint = 0;

	SetServiceStatus(g_StatusHandle, &g_ServiceStatus);

	// Start the thread that will perform the main task of the service
	HANDLE hThread = CreateThread(NULL, 0, ServiceWorkerThread, NULL, 0, NULL);

	// Wait until our worker thread exits effectively signaling that the service needs to stop
	WaitForSingleObject(hThread, INFINITE);

	CloseHandle(g_ServiceStopEvent);

	g_ServiceStatus.dwControlsAccepted = 0;
	g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
	g_ServiceStatus.dwWin32ExitCode = 0;
	g_ServiceStatus.dwCheckPoint = 3;

	SetServiceStatus(g_StatusHandle, &g_ServiceStatus);

EXIT:
	return;
}


VOID WINAPI ServiceCtrlHandler(DWORD CtrlCode)
{
	switch (CtrlCode)
	{
	case SERVICE_CONTROL_STOP:

		if (g_ServiceStatus.dwCurrentState != SERVICE_RUNNING)
			break;

		g_ServiceStatus.dwControlsAccepted = 0;
		g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
		g_ServiceStatus.dwWin32ExitCode = 0;
		g_ServiceStatus.dwCheckPoint = 4;

		SetServiceStatus(g_StatusHandle, &g_ServiceStatus);

		SetEvent(g_ServiceStopEvent);

		break;

	default:
		break;
	}
}