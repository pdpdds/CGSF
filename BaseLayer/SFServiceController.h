#pragma once

class SFServiceController
{
public:
	SFServiceController(void);
	virtual ~SFServiceController(void);

	BOOL InstallService(TCHAR* szServiceName, TCHAR* szServiceDescription, TCHAR* szServicePath);
	BOOL DeleteService( TCHAR* szServiceName);
	BOOL StartService(TCHAR* szServiceName);
	BOOL StopService(TCHAR* szServiceName);
};
