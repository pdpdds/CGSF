/*!
	\file DiskManager.h
    \class CDiskManager
    \brief A Disk Management class to check disk information like total size, free space, used space, and clean up.
	\note Special acknowledgement to Hans Dietrich on XFolderSize.cpp
	\n Revision:
	\n Date         Name                Changes
	\n 22-Jun-09    Low Weng Seng       Initial Creation.
*/
#include <windows.h>

typedef BOOL (WINAPI *P_GFS)(HANDLE, PLARGE_INTEGER);

class CDiskManager
{
public:
	CDiskManager();
	virtual ~CDiskManager();

public:
	/*!
		\fn BOOL GetFileSize(LPCTSTR szPath, PLARGE_INTEGER lpFileSize)
		\brief Get file size(64-bit).
		\param szPath Fully qualified path to file.
		\param lpFileSize An unsigned integer pointer to store total size value in Byte.
		\return BOOL - Returns nonzero if successful or zero otherwise.
	*/
	virtual BOOL GetFileSize(LPCTSTR szPath, PLARGE_INTEGER lpFileSize);
	/*!
		\fn BOOL GetFolderSize(LPCTSTR szFolderPath, BOOL bRecursive, BOOL bQuickSize, 
			PLARGE_INTEGER lpFolderSize, LPDWORD lpFolderCount = NULL, LPDWORD lpFileCount = NULL)
		\brief Get folder size.
		\param szFolderPath Fully qualified starting folder path.
		\param bRecursive TRUE = recurse into subdirectories.
		\param bQuickSize TRUE = if szPath is a logical drive(root) - folder count and file count will be 0.
		\param lpFolderSize Pointer to 64-bit folder size.
		\param lpFolderCount Pointer to 32-bit folder count (optional, may be NULL)
		\param lpFileCount Pointer to 32-bit file count (optional, may be NULL)
		\return BOOL - Returns nonzero if successful or zero otherwise.
	*/
	virtual BOOL GetFolderSize(LPCTSTR szFolderPath, 
		BOOL bRecursive, 
		BOOL bQuickSize, 
		PLARGE_INTEGER lpFolderSize,
		LPDWORD lpFolderCount = NULL,
		LPDWORD lpFileCount = NULL);

	/*!
		\fn BOOL CleanupFolder(LPCTSTR szPath, LPDWORD lpFileCount = NULL)
		\brief Delete the oldest folder recusively from given path.
		\param szPath Fully qualified path to file.
		\param lpFileCount Pointer to 32-bit folder count (optional, may be NULL).
		\return BOOL - Returns nonzero if successful or zero otherwise.
	*/
	virtual BOOL CleanupFolder(LPCTSTR szPath, LPDWORD lpFileCount = NULL);

	/*!
		\fn BOOL GetDiskDetails(LPCTSTR szDrivePath, PULARGE_INTEGER lpFreeSpace, 
			PULARGE_INTEGER lpTotalSize)
		\brief Retrieve the details of a local disk
		\param lpFreeSpace An unsigned integer pointer to store free space value in MB
		\param lpTotalSize An unsigned integer pointer to store total size value in MB
		\return BOOL - Returns nonzero if successful or zero otherwise.
	*/
	virtual BOOL GetDiskDetails(LPCTSTR szDrivePath, PULARGE_INTEGER lpFreeSpace, 
		PULARGE_INTEGER lpTotalSize);

	/*!
		\var P_GFS pGetFileSizeEx
		\brief WINAPI pointer to GetFileSizeEx function
		\note Since GetFileSizeEx is not in Windows 95 Retail, we
        dynamically link to it and only call it if it is present.  We 
        don't need to call LoadLibrary on KERNEL32.DLL because it is 
        already loaded into every Win32 process's address space.
		\n It is declared as member variable for optimization purpose due to it will be called recursively if there is sub-forlder.
	*/
	P_GFS pGetFileSizeEx;
};