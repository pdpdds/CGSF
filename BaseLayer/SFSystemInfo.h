#pragma once
#include "CPUDesc.h"

struct OSInfo
{
	std::string OperatingSystem;
	bool isServer = false;
	int MajorVer = 0;
	int MinorVer = 0;
};

class SFSystemInfo
{
public:
	SFSystemInfo(void);
	virtual ~SFSystemInfo(void);

	bool Initialize();

	CHAR* GetGraphicCardName();

	int GetUploadBandWidth();
	int GetDownloadBandWidth();

	CCPUDesc* GetCPUDesc(){ return m_pCPUDesc.get(); }
	OSInfo* GetOSInfo() { return &m_OSInfo; }
	MEMORYSTATUS* GetMemoryInfo(){ return &m_MemoryStatus; }

protected:	
	bool InitializeCPUInfo();
	bool InitializeMemoryInfo();
	bool InitializeOSInfo();
	

private:	
	std::shared_ptr<CCPUDesc> m_pCPUDesc;
	OSInfo m_OSInfo;
	MEMORYSTATUS m_MemoryStatus;
};