#include "StdAfx.h"
#include "SFSystemInfo.h"
#include "stdio.h"
#include "CPUDesc.h"


SFSystemInfo::SFSystemInfo(void)
: m_pCPUDesc(NULL)

{
	memset(&m_MemoryStatus, 0, sizeof(MEMORYSTATUS));
	m_OSInfo = new OSInfo;
}

SFSystemInfo::~SFSystemInfo(void)
{
	if(m_pCPUDesc)
	{
		delete m_pCPUDesc;
		m_pCPUDesc = NULL;
	}

	delete m_OSInfo;
}

BOOL SFSystemInfo::Initialize()
{
	if(FALSE == InitializeCPUInfo())
		return FALSE;

	if(FALSE == InitializeOSInfo())
		return FALSE;

	if(FALSE == InitializeMemoryInfo())
		return FALSE;

	return TRUE;
}

BOOL SFSystemInfo::InitializeCPUInfo()
{
	if(NULL != m_pCPUDesc)
		return FALSE;

	m_pCPUDesc = new CCPUDesc ();

	if(FALSE == m_pCPUDesc->Initialize())
		return FALSE;

	return TRUE;
}

BOOL SFSystemInfo::InitializeMemoryInfo()
{	
	memset(&m_MemoryStatus, 0, sizeof(MEMORYSTATUS));
    m_MemoryStatus.dwLength= sizeof(MEMORYSTATUS);

    GlobalMemoryStatus(&m_MemoryStatus);
    
	return TRUE;
}


BOOL SFSystemInfo::InitializeOSInfo()
{
	OSVERSIONINFOEX osvi;
	BOOL bIsWindows64Bit;
	BOOL bOsVersionInfoEx;

	char szOperatingSystem[256];
	memset(szOperatingSystem, 0, sizeof(char) * 256);

	// Try calling GetVersionEx using the OSVERSIONINFOEX structure.
	// If that fails, try using the OSVERSIONINFO structure.
	ZeroMemory (&osvi, sizeof (OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFOEX);

	if (!(bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi))) {
		// If OSVERSIONINFOEX doesn't work, try OSVERSIONINFO.
		osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
		if (!GetVersionEx ((OSVERSIONINFO *) &osvi)) return FALSE;
	}

	m_OSInfo->dwOSMajorVer = osvi.dwMajorVersion;
	m_OSInfo->dwOSMinorVer = osvi.dwMinorVersion;
	m_OSInfo->dwOSType = osvi.dwPlatformId;

	switch (osvi.dwPlatformId) {
		case VER_PLATFORM_WIN32_NT:
			// Test for the product.
			if (osvi.dwMajorVersion <= 4) strcpy_s (szOperatingSystem, "Microsoft Windows NT  ");
			if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0) strcpy_s (szOperatingSystem, "Microsoft Windows 2000 ");
			if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1) strcpy_s (szOperatingSystem, "Microsoft Windows XP ");
			if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2) strcpy_s (szOperatingSystem, "Microsoft Windows 2003 ");
			if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0) strcpy_s (szOperatingSystem, "Microsoft Windows Vista ");
			if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1) strcpy_s (szOperatingSystem, "Microsoft Window7 ");

			// Test for product type.
			if (bOsVersionInfoEx) {
				if (osvi.wProductType == VER_NT_WORKSTATION) {
					if (osvi.wSuiteMask & VER_SUITE_PERSONAL) strcat_s (szOperatingSystem, "Personal ");
					else strcat_s (szOperatingSystem, "Professional ");
				} else if (osvi.wProductType == VER_NT_SERVER) {
					// Check for .NET Server instead of Windows XP.
					if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1) strcpy_s(szOperatingSystem, "Microsoft Windows.NET ");
					
					// Continue with the type detection.
					if (osvi.wSuiteMask & VER_SUITE_DATACENTER) strcat_s (szOperatingSystem, "DataCenter Server ");
					else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE) strcat_s (szOperatingSystem, "Advanced Server ");
					else strcat_s (szOperatingSystem, "Server ");
				}
			} else {
				HKEY hKey;
				char szProductType[80];
				DWORD dwBufLen;

				// Query the registry to retrieve information.
				RegOpenKeyExA (HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\ProductOptions", 0, KEY_QUERY_VALUE, &hKey);
				RegQueryValueExA (hKey, "ProductType", NULL, NULL, (LPBYTE) szProductType, &dwBufLen);
				RegCloseKey (hKey);
				if (lstrcmpiA ("WINNT", szProductType) == 0) strcat_s (szOperatingSystem, "Professional ");
				if (lstrcmpiA ("LANMANNT", szProductType) == 0)
					// Decide between Windows 2000 Advanced Server and Windows .NET Enterprise Server.
					if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1) 
						 strcat_s(szOperatingSystem, "Standard Server ");
					else strcat_s(szOperatingSystem, "Server ");
				if (lstrcmpiA ("SERVERNT", szProductType) == 0)
					// Decide between Windows 2000 Advanced Server and Windows .NET Enterprise Server.
					if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1) 
						 strcat_s(szOperatingSystem, "Enterprise Server ");
					else strcat_s(szOperatingSystem, "Advanced Server ");
			}

			// Display version, service pack (if any), and build number.
			if (osvi.dwMajorVersion <= 4) {
				// NB: NT 4.0 and earlier.
				sprintf_s (szOperatingSystem, "%sversion %d.%d %s (Build %d)",
											szOperatingSystem,
											osvi.dwMajorVersion,
											osvi.dwMinorVersion,
											osvi.szCSDVersion,
											osvi.dwBuildNumber & 0xFFFF);
			} else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1) {
				// Windows XP and .NET server.
				typedef BOOL (CALLBACK* LPFNPROC) (HANDLE, BOOL *);
				HINSTANCE hKernelDLL; 
				LPFNPROC DLLProc;
				
				// Load the Kernel32 DLL.
				hKernelDLL = LoadLibraryA ("kernel32");
				if (hKernelDLL != NULL)  { 
					// Only XP and .NET Server support IsWOW64Process so... Load dynamically!
					DLLProc = (LPFNPROC) GetProcAddress (hKernelDLL, "IsWow64Process"); 
				 
					// If the function address is valid, call the function.
					if (DLLProc != NULL) (DLLProc) (GetCurrentProcess (), &bIsWindows64Bit);
					else bIsWindows64Bit = false;
				 
					// Free the DLL module.
					FreeLibrary (hKernelDLL); 
				} 

 				// IsWow64Process ();
				if (bIsWindows64Bit) strcat_s(szOperatingSystem, "64-Bit ");
				else strcat_s(szOperatingSystem, "32-Bit ");
			} else { 
				// Windows 2000 and everything else.
				sprintf_s(szOperatingSystem, "%s%s(Build %d)", szOperatingSystem, osvi.szCSDVersion, osvi.dwBuildNumber & 0xFFFF);
			}
			break;

		case VER_PLATFORM_WIN32_WINDOWS:
			// Test for the product.
			if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0) {
				strcpy_s(szOperatingSystem, "Microsoft Windows 95 ");
				if (osvi.szCSDVersion[1] == 'C') strcat_s(szOperatingSystem, "OSR 2.5 ");
				else if (osvi.szCSDVersion[1] == 'B') strcat_s(szOperatingSystem, "OSR 2 ");
			} 

			if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10) {
				strcpy_s(szOperatingSystem, "Microsoft Windows 98 ");
				if (osvi.szCSDVersion[1] == 'A' ) strcat_s(szOperatingSystem, "SE ");
			} 

			if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 90) {
				strcpy_s(szOperatingSystem, "Microsoft Windows Me ");
			} 
			break;

		case VER_PLATFORM_WIN32s:
			strcpy_s(szOperatingSystem, "Microsoft Win32s ");
			break;

		default:
			strcpy_s(szOperatingSystem, "Unknown Windows ");
			break;
	}

	memcpy(m_OSInfo->szOperatingSystem, szOperatingSystem, sizeof(char) * 256);
	m_OSInfo->szOperatingSystem[255] = 0;	

	return TRUE; 
}