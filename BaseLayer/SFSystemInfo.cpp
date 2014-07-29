#include "StdAfx.h"
#include <VersionHelpers.h>
#include "SFSystemInfo.h"
#include "stdio.h"
#include "CPUDesc.h"


SFSystemInfo::SFSystemInfo(void)
{
	memset(&m_MemoryStatus, 0, sizeof(MEMORYSTATUS));
	//m_OSInfo = new OSInfo;
}

SFSystemInfo::~SFSystemInfo(void)
{

	/*if(m_pCPUDesc)
	{
		delete m_pCPUDesc;
		m_pCPUDesc = NULL;
	}*/

	//delete m_OSInfo;
}

bool SFSystemInfo::Initialize()
{
	if(InitializeCPUInfo() == false)
		return false;

	if(InitializeOSInfo() == false)
		return false;

	if(InitializeMemoryInfo() == false)
		return false;

	return true;
}

bool SFSystemInfo::InitializeCPUInfo()
{
	if (m_pCPUDesc != nullptr)
	{
		return false;
	}

	m_pCPUDesc = std::make_shared<CCPUDesc>();

	if (m_pCPUDesc->Initialize() == false)
	{
		return false;
	}
	/*if(NULL != m_pCPUDesc)
		return FALSE;

	m_pCPUDesc = new CCPUDesc ();

	if(FALSE == m_pCPUDesc->Initialize())
		return FALSE;*/

	return true;
}

bool SFSystemInfo::InitializeMemoryInfo()
{	
	memset(&m_MemoryStatus, 0, sizeof(MEMORYSTATUS));
    m_MemoryStatus.dwLength= sizeof(MEMORYSTATUS);

    GlobalMemoryStatus(&m_MemoryStatus);
    
	return true;
}


bool SFSystemInfo::InitializeOSInfo()
{
	// 2014.07.02 최흥배 GetVersionEx를 MS에서 비추천 API로 지정되어서 새 API로 OS의 간단한 정보만 얻었습니다.
	// MSDN의 OS 버전 번호 http://msdn.microsoft.com/en-us/library/ms724832(v=vs.85).aspx

	if (IsWindowsServer())
	{
		m_OSInfo.isServer = true;
	}

	if (IsWindows8Point1OrGreater())
	{
		if (m_OSInfo.isServer)
			m_OSInfo.szOperatingSystem = "Windows Server 2012 R2 or Grater";
		else
			m_OSInfo.szOperatingSystem = "Windows 8.1 or Grater";

		m_OSInfo.majorVer = 6;
		m_OSInfo.minorVer = 3;
	}
	else if (IsWindows8OrGreater())
	{
		if (m_OSInfo.isServer)
			m_OSInfo.szOperatingSystem = "Windows Server 2012";
		else
			m_OSInfo.szOperatingSystem = "Windows 8";

		m_OSInfo.majorVer = 6;
		m_OSInfo.minorVer = 2;
	}
	else if (IsWindows7OrGreater())
	{
		if (m_OSInfo.isServer)
			m_OSInfo.szOperatingSystem = "Windows Server 2008 R2";
		else
			m_OSInfo.szOperatingSystem = "Windows 7";

		m_OSInfo.majorVer = 6;
		m_OSInfo.minorVer = 1;
	}
	else if (IsWindowsVistaOrGreater())
	{
		if (m_OSInfo.isServer)
			m_OSInfo.szOperatingSystem = "Windows Server 2008";
		else
			m_OSInfo.szOperatingSystem = "Windows Vista";

		m_OSInfo.majorVer = 6;
		m_OSInfo.minorVer = 0;
	}
	else if (IsWindowsXPOrGreater())
	{
		if (m_OSInfo.isServer)
			m_OSInfo.szOperatingSystem = "Windows Server 2003";
		else
			m_OSInfo.szOperatingSystem = "Windows XP";

		m_OSInfo.majorVer = 5;
		m_OSInfo.minorVer = 1;
	}

	return true; 
}

