#pragma once
#include "CPUDesc.h"

typedef struct tagOSInfo
{
	char szOperatingSystem[256];
	DWORD dwOSType;
	DWORD dwOSMajorVer;
	DWORD dwOSMinorVer;

	tagOSInfo()
	{		
		memset(szOperatingSystem, 0, sizeof(char) * 256);
		dwOSType = 0;
		dwOSMajorVer = 0;
		dwOSMinorVer = 0;
	}
}OSInfo;

class SFSystemInfo
{
public:
	SFSystemInfo(void);
	virtual ~SFSystemInfo(void);

	BOOL Initialize();

	CHAR* GetGraphicCardName();

	int GetUploadBandWidth();
	int GetDownloadBandWidth();

	CCPUDesc* GetCPUDesc(){return m_pCPUDesc;}
	OSInfo* GetOSInfo(){return m_OSInfo;}
	MEMORYSTATUS* GetMemoryInfo(){return &m_MemoryStatus;}

protected:	
	BOOL InitializeCPUInfo();
	BOOL InitializeMemoryInfo();
	BOOL InitializeOSInfo();

private:	
	
	CCPUDesc* m_pCPUDesc;
	OSInfo* m_OSInfo;
	MEMORYSTATUS m_MemoryStatus;
};