#pragma once

class CPUInfo;

typedef struct tagProcessorInfo
{					
	DWORD dwProcessorIndex;

// Serial Number
	char szSerialNumber[128];

// On-Chip Hardware Features
	bool bAPICPresent;
	int APICID;
	bool bACPICapable;
	bool bOnChipThermalMonitor;
	
	bool bL1Cache;
	int  L1CacheSize;
	bool bL2Cache;
	int  L2CacheSize;
	bool bL3Cache;
	int  L3CacheSize;

//Power Management Features
	bool bTemperatureSensingDiode;
	bool bFrequencyIDControl;
	bool bVoltageIDControl;

//Supported Features
	bool bCMOVInstructions;

	bool bMTRRInstructions;
	bool bMMXInstructions;
	bool bMMXPlusInstructions;

	bool bSSEInstructions;

	bool bSSEFPInstructions;
	bool bSSEMMXInstructions;
	bool bSSE2Instructions;

	bool bAMD3DNowInstructions;
	bool bAMD3DNowPlusInstructions;

	bool bHyperthreadingInstructions;
	int LogicalProcessorsPerPhysical;

	bool bMultiprocessorCapable;
	bool bIA64Instructions;

	tagProcessorInfo()
	{				
		dwProcessorIndex = 0;
		memset(this, 0, sizeof(PROCESSORINFO));
	}

} PROCESSORINFO, *PPROCESSORINFO;

class CCPUDesc
{
	friend class SFSystemInfo;
public:	
	CCPUDesc(void);
	virtual ~CCPUDesc(void);

	char* GetVendorID();
	char* GetTypeID();
	char* GetFamilyID();
	char* GetModelID();
	char* GetSteppingCode();
	char* GetExtendedProcessorName();
	int GetProcessorClockFrequency();
	int GetProcessorNum(){return m_dwProcessorsNum;}	

	bool IsSupportCPUID(){return m_bSupportCPUID;}

	PPROCESSORINFO GetProcessorsInfo(){return m_aDetailProcessorInfo;}


	DWORD GetL1CacheSize(){return m_processorL1CacheSize;}
	DWORD GetL2CacheSize(){return m_processorL2CacheSize;}
	DWORD GetL3CacheSize(){return m_processorL3CacheSize;}

	bool ProcessGetDetailedProcessInfo();
	bool ProcessGetCacheInfo();

protected:
	bool Initialize();

private:
	bool m_bSupportCPUID;
	DWORD m_dwProcessorsNum;
	CPUInfo* m_pCPUInfo;
	PPROCESSORINFO m_aDetailProcessorInfo;

//Cache
	DWORD m_logicalProcessorCount;
    DWORD m_numaNodeCount;
    DWORD m_processorCoreCount;
    DWORD m_processorL1CacheCount;
    DWORD m_processorL2CacheCount;
    DWORD m_processorL3CacheCount;
    DWORD m_processorPackageCount;

	DWORD m_processorL1CacheSize;
	DWORD m_processorL2CacheSize;
	DWORD m_processorL3CacheSize;
};