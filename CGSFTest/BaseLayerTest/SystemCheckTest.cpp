#include "stdafx.h"
#include "SystemCheckTest.h"
#include "SFSystemInfo.h"
#include "SFUtil.h"
//////////////////////////////////////////////////////////////////////////
//SystemCheck Test
//////////////////////////////////////////////////////////////////////////

SystemCheckTest::SystemCheckTest(void)
{
}


SystemCheckTest::~SystemCheckTest(void)
{
}

bool SystemCheckTest::Run()
{
	float Score = SFUtil::GetWindowAssetPoint();

	SFSystemInfo SystemInfo;

	if(TRUE == SystemInfo.Initialize())
	{
		OSInfo* pOSInfo = SystemInfo.GetOSInfo();
		DWORD dwOSMajorVersion = pOSInfo->dwOSMajorVer;
		DWORD dwOSMinorVersion = pOSInfo->dwOSMinorVer;

		printf("##Operation System Infomation##\n");
		printf("OSName : %s, Ver : %d.%d\n", pOSInfo->szOperatingSystem, dwOSMajorVersion, dwOSMinorVersion);

		MEMORYSTATUS* pMemoryInfo = SystemInfo.GetMemoryInfo();

		DWORD dwTotalPhysicalMemory = pMemoryInfo->dwTotalPhys;
		dwTotalPhysicalMemory = dwTotalPhysicalMemory /1048576;

		DWORD dwTotalAvailablePhysicalMemory = pMemoryInfo->dwAvailPhys;
		dwTotalAvailablePhysicalMemory = dwTotalAvailablePhysicalMemory /1048576;

		printf("\n\n##MemoryInfomation##\n");
		printf("Physical Memory : %dMB\n", dwTotalPhysicalMemory);
		printf("Available Memory : %dMB\n", dwTotalAvailablePhysicalMemory);

///////////////////////////////////////////////////////////////////////////////////////////////////
//CPU Info
///////////////////////////////////////////////////////////////////////////////////////////////////
		CCPUDesc* pDesc = SystemInfo.GetCPUDesc();
		if(true == pDesc->IsSupportCPUID())
		{	
			printf("\n\n##CPU Infomation##\n");
			printf("VenderID : %s\n", pDesc->GetVendorID());
			printf("TypeID : %s\n", pDesc->GetTypeID());
			printf("FamilyID : %s\n", pDesc->GetFamilyID());
			printf("ModelID : %s\n", pDesc->GetModelID());
			printf("SteppingCode : %s\n", pDesc->GetSteppingCode());
			printf("ExtendedProcessorName : %s\n", pDesc->GetExtendedProcessorName());
			printf("GetProcessorClockFrequency : %d\n", pDesc->GetProcessorClockFrequency());
			printf("GetProcessorNum : %d\n", pDesc->GetProcessorNum());
		}
		
		PPROCESSORINFO pProcessorsInfo = pDesc->GetProcessorsInfo();

		/*for(int nProcessorCount = 0; nProcessorCount < pDesc->GetProcessorNum(); nProcessorCount++)
		{
			printf("\n\n##Processor %d Infomation##\n",  pProcessorsInfo[nProcessorCount].dwProcessorIndex);

			if(TRUE == pProcessorsInfo[nProcessorCount].bL1Cache)
			{
				printf("L1Cache : %d\n", pProcessorsInfo[nProcessorCount].L1CacheSize);
			}

			if(TRUE == pProcessorsInfo[nProcessorCount].bL2Cache)
			{
				printf("L2Cache : %d\n", pProcessorsInfo[nProcessorCount].L2CacheSize);
			}

			if(TRUE == pProcessorsInfo[nProcessorCount].bL3Cache)
			{
				printf("L3Cache : %d\n", pProcessorsInfo[nProcessorCount].L3CacheSize);
			}

			if(TRUE == pProcessorsInfo[nProcessorCount].bHyperthreadingInstructions)
			{
				printf("bHyperthreadingInstructions Enable\n");
				printf("LogicalProcessorsPerPhysical : %d\n", pProcessorsInfo[nProcessorCount].LogicalProcessorsPerPhysical);
			}	

		}*/

		DWORD dwL1CacheSize = pDesc->GetL1CacheSize();
		DWORD dwL2CacheSize = pDesc->GetL2CacheSize();
		DWORD dwL3CacheSize = pDesc->GetL3CacheSize();

		DWORD dwMB = 0;
		DWORD dwKB = 0;

		SFUtil::ConvertBytesToMB(dwL1CacheSize, dwMB, dwKB);
		_tprintf(TEXT("L1 cache size : %dMB %dKB\n"), dwMB, dwKB);

		SFUtil::ConvertBytesToMB(dwL2CacheSize, dwMB, dwKB);
		_tprintf(TEXT("L2 cache size : %dMB %dKB\n"), dwMB, dwKB);

		SFUtil::ConvertBytesToMB(dwL3CacheSize, dwMB, dwKB);
		_tprintf(TEXT("L3 cache size : %dMB %dKB\n"), dwMB, dwKB);
	}

	return true;
}


