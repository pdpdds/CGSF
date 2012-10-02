#include "stdafx.h"
#include "SFDiskManager.h"
#include <tchar.h>
#include <ShellAPI.h>


CDiskManager::CDiskManager():
pGetFileSizeEx(NULL)
{
	pGetFileSizeEx = (P_GFS)GetProcAddress(GetModuleHandle(_T("kernel32.dll")), "GetFileSizeEx");
}
CDiskManager::~CDiskManager()
{

}
BOOL CDiskManager::GetFileSize(LPCTSTR szPath, PLARGE_INTEGER lpFileSize)
{
	BOOL bValid = FALSE;
	if (szPath && lpFileSize)
	{
		lpFileSize->QuadPart = 0;
		HANDLE hFile = NULL;

		hFile = CreateFile(szPath, READ_CONTROL, 0, NULL,
			OPEN_EXISTING, 0, NULL);

		if (hFile != INVALID_HANDLE_VALUE)
		{
			if(pGetFileSizeEx)
			{
				bValid = pGetFileSizeEx(hFile, lpFileSize);
			}
			CloseHandle(hFile);
		}
	}

	return bValid;
}

BOOL CDiskManager::GetFolderSize(LPCTSTR szFolderPath, 
		BOOL bRecursive, 
		BOOL bQuickSize, 
		PLARGE_INTEGER lpFolderSize,
		LPDWORD lpFolderCount,
		LPDWORD lpFileCount)
{
	BOOL bValid = FALSE;
	if (szFolderPath && lpFolderSize)
	{
		//Initialization
		lpFolderSize->QuadPart = 0;
		if(lpFolderCount)
			*lpFolderCount = 0;
		if(lpFileCount)
			*lpFileCount = 0;

		//Check for quick size, or the path is a root(C:\ or D:\)
		if(bQuickSize && (_tcslen(szFolderPath)<4))
		{
			ULARGE_INTEGER ulFreeCaller, ulTotal, ulFree;
			ulFreeCaller.QuadPart = 0;
			ulTotal.QuadPart = 0;
			ulFree.QuadPart = 0;
			//This function calls the GetDiskFreeSpaceEx function if it is available on the operating system. 
			//If GetDiskFreeSpaceEx is not available, it is emulated by calling the GetDiskFreeSpace function 
			//and manipulating the return values. 
			bValid = SHGetDiskFreeSpace(szFolderPath, &ulFreeCaller, &ulTotal, &ulFree);
			lpFolderSize->QuadPart=ulTotal.QuadPart - ulFree.QuadPart;
			return bValid;
		}

		HANDLE hFileFinder = 0;
		WIN32_FIND_DATA wfdata = { 0 };
		TCHAR szPath[2000] = {0};
		szPath[0] = '\0';
		//memset ( (void *)&szPath, '\0', sizeof(TCHAR) * 2000);
		LARGE_INTEGER li_total;
		li_total.QuadPart = 0;
		DWORD dwTotalFolderCount = 0;
		DWORD dwTotalFileCount = 0;

		_tcsncpy_s(szPath, szFolderPath, sizeof(szPath)/sizeof(TCHAR)-1);
		_tcscat_s(szPath, _T("\\*"));

		hFileFinder = FindFirstFile(szPath, &wfdata);
		if(hFileFinder!=INVALID_HANDLE_VALUE)
		{
			bValid = TRUE;
			do
			{
				_tcsncpy_s(szPath, szFolderPath, sizeof(szPath)/sizeof(TCHAR)-1);
				_tcscat_s(szPath, _T("\\"));
				_tcscat_s(szPath, wfdata.cFileName);

				if(wfdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)	//directory
				{
					if(_tcscmp(wfdata.cFileName, _T(".")) &&
						_tcscmp(wfdata.cFileName, _T("..")) &&
						bRecursive)
					{
						LARGE_INTEGER li_Temp;
						li_Temp.QuadPart = 0;
						DWORD dwFolderCount = 0;
						DWORD dwFileCount = 0;
						bValid = GetFolderSize(szPath, bRecursive, bQuickSize, &li_Temp, &dwFolderCount, &dwFileCount);
						li_total.QuadPart += li_Temp.QuadPart;
						dwTotalFolderCount += dwFolderCount + 1;
						dwTotalFileCount += dwFileCount;
					}
				}
				else	//files
				{
					LARGE_INTEGER li_TempSize;
					li_TempSize.QuadPart = 0;
					bValid = GetFileSize(szPath, &li_TempSize);
					li_total.QuadPart += li_TempSize.QuadPart;
					dwTotalFileCount += 1;
				}

			}while(FindNextFile(hFileFinder, &wfdata));
			FindClose(hFileFinder);
		}
		*lpFolderSize = li_total;
		if(lpFolderCount)
			*lpFolderCount = dwTotalFolderCount;
		if(lpFileCount)
			*lpFileCount = dwTotalFileCount;

	}
	return bValid;
}


//Delete the oldest directory to save up hard disk space
BOOL CDiskManager::CleanupFolder(LPCTSTR szPath, LPDWORD lpFileCount)
{
	BOOL bValid = FALSE;
	HANDLE hFileFinder;
	WIN32_FIND_DATA wfd = {0};
	WIN32_FIND_DATA wfd_target = {0};
	DWORD dwTotalFileCount = 0;

	TCHAR szParentFolder[2000] = {0};
	TCHAR szCurrentPath[2000]= {0};
	TCHAR szFilePath[2000] = {0};
	szParentFolder[0] = '\0';		//C++ Test Fix: BD-PB-OVERFNZT-1
	szCurrentPath[0] = '\0';
	szFilePath[0] = '\0';
	//memset ( (void *)&szCurrentPath, '\0', sizeof(TCHAR) * 2000);
	_tcsncpy_s(szCurrentPath, szPath, sizeof(szCurrentPath)/sizeof(TCHAR)-1);
	_tcscat_s(szCurrentPath, _T("\\*"));

	hFileFinder = FindFirstFile(szCurrentPath, &wfd);
	   
    SYSTEMTIME st;
    GetSystemTime(&st);              // gets current time
    SystemTimeToFileTime(&st, &wfd_target.ftCreationTime);  //initialize the temporarily structure to the latest time for comparison used

	if(hFileFinder!=INVALID_HANDLE_VALUE)
	{
		bValid = TRUE;
		do
		{
			//discard parent directory
			if( (_tcscmp(wfd.cFileName, _T("."))) && (_tcscmp(wfd.cFileName, _T(".."))) )
			{
				//return 1 = First file time is later than second file time.
				if(CompareFileTime(&wfd_target.ftCreationTime, &wfd.ftCreationTime)>0 && 
					(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				{					
					wfd_target = wfd;	//the older directory is found!! 
				}
			}
		}while(FindNextFile(hFileFinder, &wfd));

		FindClose(hFileFinder);
		_tcsncpy_s(szParentFolder, szPath, sizeof(szParentFolder)/sizeof(TCHAR)-1);
		_tcscat_s(szParentFolder, _T("\\"));
		_tcscat_s(szParentFolder, wfd_target.cFileName);

		_tcsncpy_s(szCurrentPath, szParentFolder, sizeof(szCurrentPath)/sizeof(TCHAR)-1);
		_tcscat_s(szCurrentPath, _T("\\*"));

		hFileFinder = FindFirstFile(szCurrentPath, &wfd);	
		if(hFileFinder!=INVALID_HANDLE_VALUE)
		{
			do
			{
				if(!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))		/**< Delete if it is a file */
				{
					_tcsncpy_s(szFilePath, szParentFolder, sizeof(szFilePath)/sizeof(TCHAR)-1);	
					_tcscat_s(szFilePath, _T("\\"));
					_tcscat_s(szFilePath, wfd.cFileName);
					_tcscat_s(szFilePath, '\0');
					DeleteFile(szFilePath);
					dwTotalFileCount++;
				}
				else	/**<	Recursively delete if it is a sub-folder	*/
				{
					if( (_tcscmp(wfd.cFileName, _T("."))) && (_tcscmp(wfd.cFileName, _T(".."))) )
					{
						DWORD dwFileCount = 0;
						CleanupFolder(szParentFolder, &dwFileCount);
						dwTotalFileCount += dwFileCount;
					}
				}
			}while(FindNextFile(hFileFinder, &wfd));
					
			int x = FindClose(hFileFinder);

			RemoveDirectory(szParentFolder);

			if(lpFileCount)
				*lpFileCount = dwTotalFileCount;	/**<	Assign the total deleted files	*/
		}
	}
	return bValid;
}

BOOL CDiskManager::GetDiskDetails(LPCTSTR szDrivePath, PULARGE_INTEGER lpFreeSpace, 
								  PULARGE_INTEGER lpTotalSize)
{
	return (SHGetDiskFreeSpace(szDrivePath, NULL, lpTotalSize, lpFreeSpace));
}