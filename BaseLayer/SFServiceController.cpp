#include "StdAfx.h"
#include "SFServiceController.h"

SERVICE_STATUS_HANDLE SFServiceController::nServiceStatusHandle; 
HANDLE SFServiceController::killServiceEvent;
BOOL SFServiceController::nServiceRunning;
DWORD SFServiceController::nServiceCurrentStatus;
HANDLE SFServiceController::hServiceThread;

LPTHREAD_START_ROUTINE SFServiceController::funcServiceMainEntry;

//////////////////////////////////////////////////////////////
//서비스용 메시지 박스 함수
//////////////////////////////////////////////////////////////
#define DECLARE_INIT_TCHAR_ARRAY(Name, Count) TCHAR Name[Count] = {TEXT('\0')};
#define DIMOF(Array) (sizeof(Array) /sizeof(Array[0]))

void ShowMessageBoxV(LPCTSTR pMessage, va_list argList)
{
	DECLARE_INIT_TCHAR_ARRAY(szMessage, 1025);
	DECLARE_INIT_TCHAR_ARRAY(szTitle, 256 + 1);

	::wvsprintf(szMessage, pMessage, argList);
	::GetModuleFileName(NULL, szTitle, DIMOF(szTitle));
	HWND hwnd = ::GetActiveWindow();
	::MessageBox(hwnd, szMessage, szTitle, MB_OK | ((NULL == hwnd) ? MB_SERVICE_NOTIFICATION : 0));
}

void ShowMessageBox(LPCTSTR pMessage, ...)
{
	va_list argList;
	va_start(argList, pMessage);

	::ShowMessageBoxV(pMessage, argList);

	va_end(argList);
}

SFServiceController::SFServiceController(void)
{
}

SFServiceController::~SFServiceController(void)
{

}

BOOL SFServiceController::InstallService( TCHAR* szServiceName, TCHAR* szServiceDescription, TCHAR* szServicePath )
{
	SC_HANDLE Service,scm;
	scm = OpenSCManager(0,0,SC_MANAGER_CREATE_SERVICE);
	if(!scm)
	{
		return FALSE;
	}
	Service=CreateService (scm, 
						   szServiceName
						   ,szServiceDescription,
						   SERVICE_ALL_ACCESS,
						   SERVICE_WIN32_OWN_PROCESS,
						   SERVICE_DEMAND_START,
						   SERVICE_ERROR_NORMAL,
						   szServicePath, 0,0,0,0,0);
	if(!Service)
	{
		CloseServiceHandle(scm);
		return FALSE;
	}

	CloseServiceHandle(Service);
	CloseServiceHandle(scm);

	return TRUE;
}

BOOL SFServiceController::DeleteService( TCHAR* szServiceName)
{
	SC_HANDLE Service,scm;
	scm = OpenSCManager(0,0,SC_MANAGER_CREATE_SERVICE);

	if(!scm)
	{
		return FALSE;
	}
	
	Service = OpenService(scm,szServiceName, SERVICE_ALL_ACCESS);
		
	if(Service)
	{
		if(::DeleteService(Service))
		{
			CloseServiceHandle(Service);
			CloseServiceHandle(scm);
			return TRUE;
		}
		else
		{
			CloseServiceHandle(Service);
			CloseServiceHandle(scm);
			return FALSE;
		}

	}
	CloseServiceHandle(scm);

	return FALSE;
}


BOOL SFServiceController::StartService( TCHAR* szServiceName )
{
	SC_HANDLE Service,scm;
	scm=OpenSCManager(0,0,SC_MANAGER_CREATE_SERVICE);

	if(!scm)
	{
		return FALSE;
	}
	Service = OpenService(scm, szServiceName, SERVICE_ALL_ACCESS);
	if(!Service)
	{
		CloseServiceHandle(scm);
		return FALSE;
	}
	
	::StartService(Service, 0, NULL);
	
	CloseServiceHandle(Service);
	CloseServiceHandle(scm);
	return TRUE;
}

BOOL SFServiceController::StopService( TCHAR* szServiceName )
{
	SC_HANDLE Service,scm;
	scm=OpenSCManager(0,0,SC_MANAGER_CREATE_SERVICE);

	if(!scm)
	{
		return FALSE;
	}
	Service = OpenService(scm, szServiceName, SERVICE_ALL_ACCESS);
	if(!Service)
	{
		CloseServiceHandle(scm);
		return FALSE;
	}

	SERVICE_STATUS m_SERVICE_STATUS;
	ControlService(Service, SERVICE_CONTROL_STOP, &m_SERVICE_STATUS);

	CloseServiceHandle(Service);
	CloseServiceHandle(scm);
	return TRUE;
}

BOOL SFServiceController::ServiceEntry(TCHAR* szServiceName, LPTHREAD_START_ROUTINE ServiceStartEntry)
{	
	funcServiceMainEntry = ServiceStartEntry;

	SERVICE_TABLE_ENTRY serviceTable[] =
	{
		{ szServiceName, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
		{ NULL, NULL }
	};

	return StartServiceCtrlDispatcher(serviceTable);
}

void SFServiceController::ServiceMain(DWORD argc, LPTSTR *argv)
{
	UNREFERENCED_PARAMETER(argc);

	BOOL success;
	nServiceStatusHandle = RegisterServiceCtrlHandler(argv[0],
		(LPHANDLER_FUNCTION)ServiceCtrlHandler);
	if(!nServiceStatusHandle)
	{
		return;
	}
	success=UpdateServiceStatus(SERVICE_START_PENDING,NO_ERROR,0,1,3000);
	if(!success)
	{
		return;
	}
	killServiceEvent=CreateEvent(0,TRUE,FALSE,0);
	if(killServiceEvent==NULL)
	{
		return;
	}
	success=UpdateServiceStatus(SERVICE_START_PENDING,NO_ERROR,0,2,1000);
	if(!success)
	{
		return;
	}
	success=StartServiceThread();
	if(!success)
	{
		return;
	}
	nServiceCurrentStatus=SERVICE_RUNNING;
	success=UpdateServiceStatus(SERVICE_RUNNING,NO_ERROR,0,0,0);
	if(!success)
	{
		return;
	}
	WaitForSingleObject(killServiceEvent,INFINITE);
	CloseHandle(killServiceEvent);
}

BOOL SFServiceController::UpdateServiceStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode,
						 DWORD dwServiceSpecificExitCode, DWORD dwCheckPoint,
						 DWORD dwWaitHint)
{
	BOOL success;
	SERVICE_STATUS nServiceStatus;
	nServiceStatus.dwServiceType=SERVICE_WIN32_OWN_PROCESS;
	nServiceStatus.dwCurrentState=dwCurrentState;
	if(dwCurrentState==SERVICE_START_PENDING)
	{
		nServiceStatus.dwControlsAccepted=0;
	}
	else
	{
		nServiceStatus.dwControlsAccepted=SERVICE_ACCEPT_STOP			
			|SERVICE_ACCEPT_SHUTDOWN;
	}
	if(dwServiceSpecificExitCode==0)
	{
		nServiceStatus.dwWin32ExitCode=dwWin32ExitCode;
	}
	else
	{
		nServiceStatus.dwWin32ExitCode=ERROR_SERVICE_SPECIFIC_ERROR;
	}
	nServiceStatus.dwServiceSpecificExitCode=dwServiceSpecificExitCode;
	nServiceStatus.dwCheckPoint=dwCheckPoint;
	nServiceStatus.dwWaitHint=dwWaitHint;

	success=SetServiceStatus(nServiceStatusHandle,&nServiceStatus);

	if(!success)
	{
		KillService();
		return success;
	}
	else
		return success;
}

BOOL SFServiceController::StartServiceThread()
{	
	DWORD id;
	hServiceThread=CreateThread(0,0,
		(LPTHREAD_START_ROUTINE)funcServiceMainEntry,
		0,0,&id);
	if(hServiceThread==0)
	{
		return false;
	}
	else
	{
		nServiceRunning=true;
		return true;
	}
}

DWORD SFServiceController::ServiceExecutionThread(LPDWORD param)
{
	UNREFERENCED_PARAMETER(param);

	while(nServiceRunning)
	{		
		Beep(450,150);
		Sleep(4000);
	}
	return 0;
}

void SFServiceController::KillService()
{
	nServiceRunning=false;
	SetEvent(killServiceEvent);
	UpdateServiceStatus(SERVICE_STOPPED,NO_ERROR,0,0,0);
}

void SFServiceController::ServiceCtrlHandler(DWORD nControlCode)
{
	BOOL success;
	switch(nControlCode)
	{	
	case SERVICE_CONTROL_SHUTDOWN:
	case SERVICE_CONTROL_STOP:
		nServiceCurrentStatus=SERVICE_STOP_PENDING;
		success=UpdateServiceStatus(SERVICE_STOP_PENDING,NO_ERROR,0,1,3000);
		KillService();		
		return;
	default:
		break;
	}
	UpdateServiceStatus(nServiceCurrentStatus,NO_ERROR,0,0,0);
}