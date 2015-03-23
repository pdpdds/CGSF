#include "StdAfx.h"
#include "SFServiceController.h"

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
