#include "StdAfx.h"
#include "CPUDesc.h"
#include "cpu_info.h"

CCPUDesc::CCPUDesc(void)
: m_pCPUInfo(NULL)
, m_bSupportCPUID(false)
, m_dwProcessorsNum(0)
, m_aDetailProcessorInfo(NULL)
, m_logicalProcessorCount(0)
, m_numaNodeCount(0)
, m_processorCoreCount(0)
, m_processorL1CacheCount(0)
, m_processorL2CacheCount(0)
, m_processorL3CacheCount(0)
, m_processorPackageCount(0)
, m_processorL1CacheSize(0)
, m_processorL2CacheSize(0)
, m_processorL3CacheSize(0)
{
}

CCPUDesc::~CCPUDesc(void)
{
}

DWORD CountSetBits(ULONG_PTR bitMask)
	{
		DWORD LSHIFT = sizeof(ULONG_PTR)*8 - 1;
		DWORD bitSetCount = 0;
		ULONG_PTR bitTest = (ULONG_PTR)1 << LSHIFT;    
		DWORD i;

		for (i = 0; i <= LSHIFT; ++i)
		{
			bitSetCount += ((bitMask & bitTest)?1:0);
			bitTest/=2;
		}

		return bitSetCount;
	}

BOOL CCPUDesc::Initialize()
{
	if(NULL != m_pCPUInfo)
		return FALSE;

	m_pCPUInfo = new CPUInfo();

	if(m_pCPUInfo->DoesCPUSupportCPUID())
	{
		m_bSupportCPUID = true;
	}
		
	SYSTEM_INFO SysInfo;	
	// Get the number of processors in the system.
	ZeroMemory (&SysInfo, sizeof (SYSTEM_INFO));
	GetSystemInfo (&SysInfo);
	
	// Number of physical processors in a non-Intel system
	// or in a 32-bit Intel system with Hyper-Threading technology disabled
    m_dwProcessorsNum = SysInfo.dwNumberOfProcessors;

	if(m_dwProcessorsNum)
	{
		if(FALSE == ProcessGetDetailedProcessInfo())
			return FALSE;
	}

	ProcessGetCacheInfo();

	return TRUE;
}

char* CCPUDesc::GetVendorID()
{
	return m_pCPUInfo->GetVendorID();
}

char* CCPUDesc::GetTypeID()
{
	return m_pCPUInfo->GetTypeID();
}
char* CCPUDesc::GetFamilyID()
{
	return m_pCPUInfo->GetFamilyID();
}

char* CCPUDesc::GetModelID()
{
	return m_pCPUInfo->GetModelID();
}

char* CCPUDesc::GetSteppingCode()
{
	return m_pCPUInfo->GetSteppingCode();
}

char* CCPUDesc::GetExtendedProcessorName()
{
	return m_pCPUInfo->GetExtendedProcessorName();
}

int CCPUDesc::GetProcessorClockFrequency()
{
	return m_pCPUInfo->GetProcessorClockFrequency();
}



			
//sprintf (szText, "Processor %d", ThreadInfo->nProcessor);
//sprintf (szText, "Processor Name: %s", CPU->GetExtendedProcessorName ());
//sprintf (szText, "Clock Frequency: %d MHz", CPU->GetProcessorClockFrequency ());

DWORD WINAPI GetProcessorInformation (LPVOID lpParameter)
{
	PPROCESSORINFO pProcessorInfo = (PPROCESSORINFO) lpParameter;
	CPUInfo * CPU = new CPUInfo ();
				
	if (!CPU->DoesCPUSupportCPUID ()) 
	{			
		delete CPU;			
		return 0;
	}
		
	//the processor serial number.
	if (CPU->DoesCPUSupportFeature (SERIALNUMBER_FEATURE)) 
	{
		sprintf_s(pProcessorInfo->szSerialNumber, CPU->GetProcessorSerialNumber());
	}
			
	// Add the hardware specification node.		
	// State the APIC ID if one is present.
	if (CPU->DoesCPUSupportFeature (APIC_FEATURE)) 
	{				
		pProcessorInfo->bAPICPresent = TRUE;		
		// Attempt to display the ID of the APIC.
		pProcessorInfo->APICID = CPU->GetProcessorAPICID ();
	}

	// State if the processor supports the Advanced Configuration and Power Interface [ACPI].
	if (CPU->DoesCPUSupportFeature (ACPI_FEATURE)) 
	{
		pProcessorInfo->bACPICapable = TRUE;
	}
			
	// State if the processor supports a thermal monitor.
	if (CPU->DoesCPUSupportFeature (THERMALMONITOR_FEATURE))
	{
		pProcessorInfo->bOnChipThermalMonitor = TRUE;
	}

	
	if (CPU->DoesCPUSupportFeature (L1CACHE_FEATURE)) 
	{
		pProcessorInfo->bL1Cache = TRUE;
		pProcessorInfo->L1CacheSize = CPU->GetProcessorCacheXSize (L1CACHE_FEATURE);
	}

	// State the size of the L2 cache if present.
	if (CPU->DoesCPUSupportFeature (L2CACHE_FEATURE)) 
	{
		pProcessorInfo->bL2Cache = TRUE;
		pProcessorInfo->L2CacheSize = CPU->GetProcessorCacheXSize (L2CACHE_FEATURE);
	}

	// State the size of the L3 cache if present.
	if (CPU->DoesCPUSupportFeature (L3CACHE_FEATURE)) 
	{
		pProcessorInfo->bL3Cache = TRUE;
		pProcessorInfo->L3CacheSize = CPU->GetProcessorCacheXSize (L3CACHE_FEATURE);
	}
							
	// State if a temperature sensing diode is present.
	if (CPU->DoesCPUSupportFeature (TEMPSENSEDIODE_FEATURE)) 
	{
		pProcessorInfo->bTemperatureSensingDiode = true;
	}

	// State if a frequency ID control is present.
	if (CPU->DoesCPUSupportFeature (FREQUENCYID_FEATURE)) 
	{				
		pProcessorInfo->bFrequencyIDControl = true;	
	}

	// State if a voltage ID control is present.
	if (CPU->DoesCPUSupportFeature (VOLTAGEID_FREQUENCY)) 
	{
		pProcessorInfo->bVoltageIDControl = true;
	}
	
//Supported Features
	
	// State if CMOV instructions are present.
	if (CPU->DoesCPUSupportFeature (CMOV_FEATURE)) 
	{
		pProcessorInfo->bCMOVInstructions = TRUE;
	}

	// State if MTRR instructions are present.
	if (CPU->DoesCPUSupportFeature (MTRR_FEATURE)) 
	{	
		pProcessorInfo->bMTRRInstructions = TRUE;
	}

	// State if MMX instructions are present.
	if (CPU->DoesCPUSupportFeature (MMX_FEATURE)) 
	{
		pProcessorInfo->bMMXInstructions = TRUE;
	}

	// State if MMX+ instructions are present.
	if (CPU->DoesCPUSupportFeature (MMX_PLUS_FEATURE)) 
	{		
		pProcessorInfo->bMMXPlusInstructions = TRUE;
	}

	// State if SSE instructions are present.
	if (CPU->DoesCPUSupportFeature (SSE_FEATURE)) 
	{
		pProcessorInfo->bSSEInstructions = TRUE;
	}

	// State if SSE FP instructions are present.
	if (CPU->DoesCPUSupportFeature (SSE_FP_FEATURE)) 
	{
		pProcessorInfo->bSSEFPInstructions = TRUE;
	}

	// State if SSE MMX instructions are present.
	if (CPU->DoesCPUSupportFeature (SSE_MMX_FEATURE)) 
	{
		pProcessorInfo->bMMXInstructions = TRUE;
	}

	// State if SSE2 instructions are present.
	if (CPU->DoesCPUSupportFeature (SSE2_FEATURE)) 
	{
		pProcessorInfo->bSSE2Instructions = TRUE;
	}

	// State if 3DNow! instructions are present.
	if (CPU->DoesCPUSupportFeature (AMD_3DNOW_FEATURE)) 
	{
		pProcessorInfo->bAMD3DNowInstructions = TRUE;
	}

	// State if 3DNow!+ instructions are present.
	if (CPU->DoesCPUSupportFeature (AMD_3DNOW_PLUS_FEATURE)) 
	{
		pProcessorInfo->bAMD3DNowPlusInstructions = TRUE;
	}

	// State if Hyperthreading instructions are present.
	if (CPU->DoesCPUSupportFeature (HYPERTHREAD_FEATURE)) 
	{
		pProcessorInfo->bHyperthreadingInstructions = TRUE;
		pProcessorInfo->LogicalProcessorsPerPhysical = CPU->GetLogicalProcessorsPerPhysical ();
	}

	// State if the processor is MP capable.
	if (CPU->DoesCPUSupportFeature (MP_CAPABLE)) 
	{
		pProcessorInfo->bMultiprocessorCapable = TRUE;
	}

	// State if IA64 instructions are present.
	if (CPU->DoesCPUSupportFeature (IA64_FEATURE)) 
	{
		pProcessorInfo->bIA64Instructions = TRUE;
	}	
	
	delete CPU;

	return 0;	
}

BOOL CCPUDesc::ProcessGetDetailedProcessInfo()
{
	// For each processor; spawn a CPU thread to access details.

	if (0 == m_dwProcessorsNum)
		return FALSE;

	if (NULL != m_aDetailProcessorInfo)
		return FALSE;

	m_aDetailProcessorInfo = new PROCESSORINFO[m_dwProcessorsNum];
	HANDLE * ahThread = NULL;
	DWORD * aThreadID = NULL;

	ahThread = new HANDLE[m_dwProcessorsNum];
	aThreadID = new DWORD[m_dwProcessorsNum];

	if (NULL == m_aDetailProcessorInfo ||
		NULL == ahThread ||
		NULL == aThreadID)
		return FALSE;

	for (DWORD dwCounter = 0; dwCounter < m_dwProcessorsNum; dwCounter ++) 
	{				
		m_aDetailProcessorInfo[dwCounter].dwProcessorIndex = dwCounter + 1;

		ahThread[dwCounter] = CreateThread (NULL, 
											0,												
											(LPTHREAD_START_ROUTINE) GetProcessorInformation,
											(LPVOID) &m_aDetailProcessorInfo[dwCounter], CREATE_SUSPENDED,
											&(aThreadID[dwCounter]));

		if (ahThread[dwCounter] == NULL)
		{
			return FALSE;
		}
		
		// Set the threads affinity to the correct processor.
		if (SetThreadAffinityMask (ahThread[dwCounter], dwCounter + 1) == 0) 
		{
			return FALSE;
		}

		ResumeThread (ahThread[dwCounter]);
	}

	WaitForMultipleObjects(m_dwProcessorsNum, ahThread, TRUE, INFINITE);

	for (DWORD dwCounter = 0; dwCounter < m_dwProcessorsNum; dwCounter ++) 
	{
		CloseHandle(ahThread[dwCounter]);	
	}

	delete ahThread;
	delete aThreadID;
	 
	return TRUE;
}

typedef BOOL (WINAPI *LPFN_GLPI)(
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION, 
    PDWORD);


BOOL CCPUDesc::ProcessGetCacheInfo()
{
	LPFN_GLPI glpi;
    BOOL done = FALSE;
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = NULL;
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = NULL;
    DWORD returnLength = 0;    
    DWORD byteOffset = 0;
    PCACHE_DESCRIPTOR Cache;

    glpi = (LPFN_GLPI) GetProcAddress(
                            GetModuleHandle(TEXT("kernel32")),
                            "GetLogicalProcessorInformation");
    if (NULL == glpi) 
    {
        //_tprintf(TEXT("\nGetLogicalProcessorInformation is not supported.\n"));
        return FALSE;
    }

    while (!done)
    {
        DWORD rc = glpi(buffer, &returnLength);

        if (FALSE == rc) 
        {
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) 
            {
                if (buffer) 
                    free(buffer);

                buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(
                        returnLength);

                if (NULL == buffer) 
                {
                    //_tprintf(TEXT("\nError: Allocation failure\n"));
                    return FALSE;
                }
            } 
            else 
            {
               // _tprintf(TEXT("\nError %d\n"), GetLastError());
                return FALSE;
            }
        } 
        else
        {
            done = TRUE;
        }
    }

    ptr = buffer;

    while (byteOffset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= returnLength) 
    {
        switch (ptr->Relationship) 
        {
        case RelationNumaNode:
            // Non-NUMA systems report a single record of this type.
            m_numaNodeCount++;
            break;

        case RelationProcessorCore:
            m_processorCoreCount++;

            // A hyperthreaded core supplies more than one logical processor.
			m_logicalProcessorCount += CountSetBits(ptr->ProcessorMask);
            break;

        case RelationCache:
            // Cache data is in ptr->Cache, one CACHE_DESCRIPTOR structure for each cache. 
            Cache = &ptr->Cache;
            if (Cache->Level == 1)
            {
                m_processorL1CacheCount++;
				m_processorL1CacheSize += Cache->Size;
            }
            else if (Cache->Level == 2)
            {
                m_processorL2CacheCount++;
				m_processorL2CacheSize += Cache->Size;
            }
            else if (Cache->Level == 3)
            {
                m_processorL3CacheCount++;
				m_processorL3CacheSize += Cache->Size;
            }
            break;

        case RelationProcessorPackage:
            // Logical processors share a physical package.
            m_processorPackageCount++;
            break;

        default:
            //_tprintf(TEXT("\nError: Unsupported LOGICAL_PROCESSOR_RELATIONSHIP value.\n"));
            break;
        }
        byteOffset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
        ptr++;
    }    
    
    free(buffer);

	return TRUE;
}