#include "StdAfx.h"
#include "SFFilePack.h"
#include <tchar.h>
#include <stdlib.h>

//////////////////////////////////////////////////////////////////
//지정한 디렉토리의 하위 폴더까지 포함하여 zip 파일을 생성한다.
//유니코드는 문제가 있는 것 같다. Ansi 버전으로
//////////////////////////////////////////////////////////////////	
BOOL SFFilePack::Initialize()
{
	// Open the LiteZip.DLL. Note: If LiteZip.dll is not placed in a path that can be found
	// by this app, then LoadLibrary will fail. So, either copy LiteZip.dll to the same
	// directory as this EXE, or to some directory that Windows is set to search.
	m_zipDll = (HMODULE)LoadLibrary(_T("./LiteZip.dll"));
	m_unzipDll = (HMODULE)LoadLibrary(_T("LiteUnzip.dll"));
	if (m_zipDll == 0 || m_unzipDll == 0)
	{
		show_errmsg();
		return FALSE;
	}
	// Get the addresses of 4 functions in LiteZip.dll -- ZipCreateFile(), ZipAddDir()
	// ZipClose(), and ZipFormatMessage.
	lpZipCreateFile = (ZipCreateFilePtr *)GetProcAddress(m_zipDll, ZIPCREATEFILENAME);
	lpZipAddDir = (ZipAddDirPtr *)GetProcAddress(m_zipDll, ZIPADDDIRNAME);
	lpZipClose = (ZipClosePtr *)GetProcAddress(m_zipDll, ZIPCLOSENAME);
	lpZipFormatMessage = (ZipFormatMessagePtr *)GetProcAddress(m_zipDll, ZIPFORMATMESSAGENAME);
	lpZipCreateBuffer = (ZipCreateBufferPtr *)GetProcAddress(m_zipDll, ZIPCREATEBUFFERNAME);
	lpZipAddBufferRaw = (ZipAddBufferRawPtr *)GetProcAddress(m_zipDll, ZIPADDBUFFERRAWNAME);
	lpZipGetMemory = (ZipGetMemoryPtr *)GetProcAddress(m_zipDll, ZIPGETMEMORYNAME);				
	lpZipAddFile = (ZipAddFilePtr *)GetProcAddress(m_zipDll, ZIPADDFILENAME);		
	lpZipAddBuffer = (ZipAddBufferPtr *)GetProcAddress(m_zipDll, ZIPADDBUFFERNAME);		

	lpUnzipOpenFile = (UnzipOpenFilePtr *)GetProcAddress(m_unzipDll, UNZIPOPENFILENAME);
	lpUnzipGetItem = (UnzipGetItemPtr *)GetProcAddress(m_unzipDll, UNZIPGETITEMNAME);
	lpUnzipItemToBuffer = (UnzipItemToBufferPtr *)GetProcAddress(m_unzipDll, UNZIPITEMTOBUFFERNAME);
	lpUnzipClose = (UnzipClosePtr *)GetProcAddress(m_unzipDll, UNZIPCLOSENAME);
	lpUnzipFormatMessage = (UnzipFormatMessagePtr *)GetProcAddress(m_unzipDll, UNZIPFORMATMESSAGENAME);		
	lpUnzipOpenBuffer = (UnzipOpenBufferPtr *)GetProcAddress(m_unzipDll, UNZIPOPENBUFFERNAME);
	lpUnzipGetItem = (UnzipGetItemPtr *)GetProcAddress(m_unzipDll, UNZIPGETITEMNAME);		
	lpUnzipOpenBuffer = (UnzipOpenBufferPtr *)GetProcAddress(m_unzipDll, UNZIPOPENBUFFERNAME);		
	lpUnzipItemToFile = (UnzipItemToFilePtr *)GetProcAddress(m_unzipDll, UNZIPITEMTOFILENAME);
	lpUnzipFindItem = (UnzipFindItemPtr *)GetProcAddress(m_unzipDll, UNZIPFINDITEMNAME);			
	lpUnzipOpenFileRaw = (UnzipOpenFileRawPtr *)GetProcAddress(m_unzipDll, UNZIPOPENFILERAWNAME);

	return TRUE;
}

SFFilePack::SFFilePack( void )			
{
	m_zipDll = 0;			
	m_unzipDll = 0;

}

SFFilePack::~SFFilePack( void )
{
	Finally();
}

BOOL SFFilePack::Finally()
{
	if(m_zipDll != 0)			
		FreeLibrary(m_zipDll); // Free the LiteZip.DLL

	if(m_unzipDll != 0)							
		FreeLibrary(m_unzipDll); // Free the LiteUnzip.DLL

	return true;
}

bool SFFilePack::PackDir(TCHAR* pFileName, TCHAR* pDirFullPath, int DirFullPathLen )
{	
	UNREFERENCED_PARAMETER(DirFullPathLen);
	UNREFERENCED_PARAMETER(pDirFullPath);

	HZIP		hz;
	DWORD		result;

	bool bResult = false;		

	// Create a ZIP archive on disk named "test.zip".
	if (!(result = lpZipCreateFile(&hz, pFileName, 0)))
	{
		TCHAR		buffer[MAX_PATH];

		result = GetCurrentDirectory(MAX_PATH, &buffer[0]);
		lstrcpy(&buffer[result], _T("\\MyDir"));

		// Add the contents of the dir "MyDir" (in the current dir) to the ZIP, and give the
		// names a relative dir off of the current dir
		if ((result = lpZipAddDir(hz, &buffer[0], result + 1)))
		{
			lpZipClose(hz);
			goto bad;
		}

		// Here we could call ZipAddFile to add more files to the ZIP archive, or ZipAddDir
		// to add more directories. We could also call ZipAddBuffer to add the contents of
		// some memory buffer as a "file" inside the zip. Or, we could call ZipAddHandle to
		// add the contents of some open file or pipe.

		// Done adding files, so close the ZIP archive.
		lpZipClose(hz);

		bResult = true;
	}
	else
	{
		TCHAR	msg[100];

bad:		
		lpZipFormatMessage(result, msg, sizeof(msg));
		OutputDebugString(msg);
	}

	return bResult;

}

bool SFFilePack::UnPackMem( TCHAR* pZipFileName, TCHAR* pFileName )
{		
	if(pFileName == NULL ||pZipFileName == NULL)
		return false;

	bool bResult = false;

	HUNZIP		huz;
	DWORD		result;

	// Open a ZIP archive on disk named pFileName
	if (!(result = lpUnzipOpenFile(&huz, pFileName, 0)))
	{
		ZIPENTRY		ze;
		DWORD			numitems;
		unsigned char	*buffer;

		// Find out how many items are in the archive.
		ze.Index = (DWORD)-1;
		if ((result = lpUnzipGetItem(huz, &ze)))
		{
			lpUnzipClose(huz);
			goto bad;
		}

		numitems = ze.Index;

		TCHAR szDrive[_MAX_DRIVE];
		TCHAR szDirectory[_MAX_DIR];
		TCHAR szFileName[_MAX_FNAME];
		TCHAR szExtension[_MAX_EXT];

		// Unzip each item, using the name stored (in the zip) for that item.
		for (ze.Index = 0; ze.Index < numitems; ze.Index++)
		{
			// Get info about the next item
			if ((result = lpUnzipGetItem(huz, &ze))) 
			{
				lpUnzipClose(huz);
				goto bad;
			}

			if(pFileName != NULL)
			{
				_tsplitpath_s(ze.Name, szDrive, szDirectory, szFileName, szExtension );
				_stprintf_s(szFileName, _T("%s%s"), szFileName, szExtension);

				if (_tcsicmp(szFileName, pFileName) == 0)
				{	
					// Allocate a memory buffer to decompress the item
					if (!(buffer = (unsigned char*)GlobalAlloc(GMEM_FIXED, ze.UncompressedSize)))
					{
						TCHAR msg[160];

						msg[0] = 0;
						FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), &msg[0], 160, 0);
						OutputDebugString(msg);
						break;
					}

					// Decompress the item into our buffer
					if ((result = lpUnzipItemToBuffer(huz, buffer, ze.UncompressedSize, &ze)))
					{
						GlobalFree(buffer);
						lpUnzipClose(huz);
						goto bad;
					}
				}

				continue;						
			}										

			// Allocate a memory buffer to decompress the item
			/*if (!(buffer = (unsigned char*)GlobalAlloc(GMEM_FIXED, ze.UncompressedSize)))
			{
			TCHAR msg[160];

			msg[0] = 0;
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), &msg[0], 160, 0);
			OutputDebugString(msg);
			break;
			}

			// Decompress the item into our buffer
			if ((result = lpUnzipItemToBuffer(huz, buffer, ze.UncompressedSize, &ze)))
			{
			GlobalFree(buffer);

			bad2:					
			lpUnzipClose(huz);
			goto bad;
			}*/

			// Here we would do something with the contents of buffer. It contains
			// ze.UncompressedSize bytes. We'll just display it to the console.
			//WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), buffer, ze.UncompressedSize, &ze.CompressedSize, 0);

			// We have no further use for the buffer, so we must free it.
			//GlobalFree(buffer);

		}

		bResult = true;

		// Done unzipping files, so close the ZIP archive.
		lpUnzipClose(huz);
	}
	else
	{
		TCHAR	msg[100];
bad:		
		lpUnzipFormatMessage(result, msg, sizeof(msg));
		OutputDebugString(msg);
	}		

	return bResult;
}	

void SFFilePack::show_errmsg(void)
{
	TCHAR buffer[160];

	buffer[0] = 0;
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), &buffer[0], 160, 0);
	OutputDebugString(buffer);
}

bool SFFilePack::PackMemoryRaw(TCHAR* pZipFileName, unsigned char* pBuffer, int UnCompressedSize, int& CompressedSize)
{
	bool bResult = false;		

	// Here is some data in memory which we'll compress into our zip file
	//const TCHAR Data[] =	_T("This is a test of LiteZip.dll's raw mode.\r\n\r\nHopefully this worked!\r\n");

	/************************** main() *************************
	* Program entry point.
	*/

	HZIP		hz;
	DWORD		result;

	// Create a ZIP archive in a memory buffer. Let LiteZip.dll allocate growable memory
	// from the memory pool. We'll set an upper limit of 100,000 bytes on this growable
	// memory. NOTE: This limit is not applicable to Linux
	if (!(result = lpZipCreateBuffer(&hz, 0, 100000, 0)))
	{
		unsigned char	*buffer;
		DWORD			len;
		HANDLE			base;

		// Add the contents of the Data[] memory buffer to the ZIP
		if ((result = lpZipAddBufferRaw(hz, pBuffer, UnCompressedSize)))
		{
			lpZipClose(hz);
			goto bad;
		}

		// We're done adding files to our ZIP in memory, so let's get a pointer to that
		// memory, and the final size of it. NOTE: We tell LiteZip.dll to give us the
		// memory, and we'll free it when we're done.
		if ((result = lpZipGetMemory(hz, (void **)&buffer, &len, &base))) goto bad;

		// Let's write out the zip memory-buffer to a disk file named "test.zip"
		{
			HANDLE	handle;
			DWORD	written;

			handle = CreateFile(pZipFileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
			if (handle == INVALID_HANDLE_VALUE) show_errmsg();
			else
			{
				if (!WriteFile(handle, buffer, len, &written, 0)) 
					show_errmsg();
				else
				{
					CompressedSize = len;
					bResult = true;
				}

				CloseHandle(handle);
			}
		}

		// Free the memory now that we're done with it.
		UnmapViewOfFile(buffer);
		CloseHandle(base);

	}
	else
	{
		TCHAR	msg[100];

bad:		
		lpZipFormatMessage(result, msg, sizeof(msg));

		OutputDebugString(msg);

	}			

	return bResult;

}	

bool SFFilePack::PackMemory(TCHAR* pZipFileName, TCHAR* pFileName, unsigned char* pBuffer, int BufferSize)
{
	bool bResult = false;				
	HZIP		hz;
	DWORD		result;

	// Create a ZIP archive in a memory buffer. Let LiteZip.dll allocate growable memory
	// from the memory pool. We'll set an upper limit of 100,000 bytes on this growable
	// memory. NOTE: This limit is not applicable to Linux
	if (!(result = lpZipCreateBuffer(&hz, 0, 100000, 0)))
	{
		unsigned char	*buffer;
		DWORD			len;
		HANDLE			base;

		// Add the contents of the Data1[] memory buffer to the ZIP, and give it the
		// name "data1.txt".
		if ((result = lpZipAddBuffer(hz, pFileName,pBuffer, BufferSize)))
		{
			lpZipClose(hz);
			goto bad;
		}

		// Here we could call ZipAddFile to add disk files to the ZIP archive. We could
		// also call ZipAddBuffer to add the contents of another memory buffer as a "file"
		// inside the zip. Or, we could call ZipAddHandle to add the contents of some open
		// file or pipe.

		// We're done adding files to our ZIP in memory, so let's get a pointer to that
		// memory, and the final size of it. NOTE: We tell LiteZip.dll to give us the
		// memory, and we'll free it when we're done.
		if ((result = lpZipGetMemory(hz, (void **)&buffer, &len, &base))) goto bad;

		// Let's write out the zip memory-buffer to a disk file named "test.zip"
		{
			HANDLE	handle;
			DWORD	written;

			handle = CreateFile(pZipFileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
			if (handle == INVALID_HANDLE_VALUE) show_errmsg();
			else
			{
				if (!WriteFile(handle, buffer, len, &written, 0)) show_errmsg();

				CloseHandle(handle);
			}
		}

		// Free the memory now that we're done with it.
		UnmapViewOfFile(buffer);
		CloseHandle(base);

	}
	else
	{
		TCHAR	msg[100];

bad:
		lpZipFormatMessage(result, msg, sizeof(msg));
		OutputDebugString(msg);

	}

	return bResult;
}

bool SFFilePack::PackFile( TCHAR* pZipFileName, TCHAR* pTargetFileName, bool bNewZipFile)
{
	bool bResult = false;		
	HZIP		hz;
	DWORD		result;

	if(bNewZipFile == true)
		result = lpZipCreateFile(&hz, pZipFileName, 0);
	else
		result = lpZipCreateFile(&hz, pZipFileName, 0);

	if (!result)
	{
		// Add the file "test.jpg" to the ZIP, and give it the same name inside the ZIP.
		if ((result = lpZipAddFile(hz, pTargetFileName, pTargetFileName)))
		{
			lpZipClose(hz);
			goto bad;
		}

		lpZipClose(hz);
	}
	else
	{
		TCHAR	msg[100];

bad:
		lpZipFormatMessage(result, msg, sizeof(msg));
		OutputDebugString(msg);					
	}

	return bResult;
}

bool SFFilePack::UnPackFile( TCHAR* pZipFileName, TCHAR* pExtractedFileName)
{
	bool bResult = false;		

	HUNZIP		huz;
	DWORD		result;

	if (!(result = lpUnzipOpenFile(&huz, pZipFileName, 0)))
	{
		ZIPENTRY	ze;
		DWORD		numitems;

		// Find out how many items are in the archive.
		ze.Index = (DWORD)-1;
		if ((result = lpUnzipGetItem(huz, &ze))) goto bad2;
		numitems = ze.Index;


		if(pExtractedFileName == NULL)
		{
			// Unzip each item, using the name stored (in the zip) for that item.
			for (ze.Index = 0; ze.Index < numitems; ze.Index++)
			{
				if ((result = lpUnzipGetItem(huz, &ze)) || (result = lpUnzipItemToFile(huz, ze.Name, &ze)))
				{
bad2:				
					lpUnzipClose(huz);
					goto bad;
				}
			}

			bResult = true;

			// Done unzipping files, so close the ZIP archive.
			lpUnzipClose(huz);				
		}
		else
		{
			TCHAR szDrive[_MAX_DRIVE];
			TCHAR szDirectory[_MAX_DIR];
			TCHAR szFileName[_MAX_FNAME];
			TCHAR szExtension[_MAX_EXT];

			for (ze.Index = 0; ze.Index < numitems; ze.Index++)
			{
				if (result = lpUnzipGetItem(huz, &ze))
				{
					lpUnzipClose(huz);
					goto bad;							
				}
				else
				{
					_tsplitpath_s(ze.Name, szDrive, szDirectory, szFileName, szExtension );

					_stprintf_s(szFileName, _T("%s%s"), szFileName, szExtension);

					if (_tcsicmp(szFileName, pExtractedFileName) == 0)
					{
						if(result = lpUnzipItemToFile(huz, ze.Name, &ze))
						{
							lpUnzipClose(huz);
							goto bad;									
						}
						else
						{
							bResult = true;
							break;

						}
					}
				}
			}

			lpUnzipClose(huz);
		}				
	}
	else
	{
		TCHAR	msg[100];

bad:	
		lpUnzipFormatMessage(result, msg, sizeof(msg));
		OutputDebugString(msg);

	}


	return bResult;
}

bool SFFilePack::UnPackRes(TCHAR* pExtractedFileName /*= NULL*/ )
{
	bool bResult = false;		

	HUNZIP		huz;
	DWORD		result;

	// Load/Open the ZIP archive, which is in our EXE's resources. It is an RT_RCDATA type
	// of resource with an ID number of 1.
	if (!(result = lpUnzipOpenBuffer(&huz, 0, 1, 0)))
	{
		ZIPENTRY		ze;
		DWORD			numitems;

		// Find out how many items are in the archive.
		ze.Index = (DWORD)-1;
		if ((result = lpUnzipGetItem(huz, &ze))) goto bad2;
		numitems = ze.Index;

		TCHAR szDrive[_MAX_DRIVE];
		TCHAR szDirectory[_MAX_DIR];
		TCHAR szFileName[_MAX_FNAME];
		TCHAR szExtension[_MAX_EXT];

		// Unzip each item, using the name stored (in the zip) for that item.
		for (ze.Index = 0; ze.Index < numitems; ze.Index++)
		{
			// Get info about the next item
			if ((result = lpUnzipGetItem(huz, &ze))) goto bad2;
			{

				if(pExtractedFileName != NULL)
				{
					_tsplitpath_s(ze.Name, szDrive, szDirectory, szFileName, szExtension );

					_stprintf_s(szFileName, _T("%s%s"), szFileName, szExtension);

					if (_tcsicmp(szFileName, pExtractedFileName) == 0)
					{
						// Decompress the item to a file by the same name
						if ((result = lpUnzipItemToFile(huz, ze.Name, &ze)))
						{
							lpUnzipClose(huz);
							goto bad;
						}
						else
						{
							bResult = true;
							break;
						}

					}

				}						
				else
				{
					// Decompress the item to a file by the same name
					if ((result = lpUnzipItemToFile(huz, ze.Name, &ze)))
					{
bad2:					
						lpUnzipClose(huz);
						goto bad;
					}

				}						
			}
		}

		// Done unzipping files, so close the ZIP archive.
		lpUnzipClose(huz);
	}
	else
	{
		TCHAR	msg[100];
bad:		
		lpUnzipFormatMessage(result, msg, sizeof(msg));
		OutputDebugString(msg);
	}

	return bResult;
}

bool SFFilePack::UnPackResMemAll( TCHAR* pResourceName)
{		
	UNREFERENCED_PARAMETER(pResourceName);

	HUNZIP		huz;
	DWORD		result;
	bool bResult = false;

	// Load/Open the ZIP archive, which is in our EXE's resources. It is an RT_RCDATA type
	// of resource with an ID number of 1.
	if (!(result = lpUnzipOpenBuffer(&huz, 0, 1, 0)))
	{
		ZIPENTRY		ze;
		DWORD			numitems;

		// Find out how many items are in the archive.
		ze.Index = (DWORD)-1;
		if ((result = lpUnzipGetItem(huz, &ze))) goto bad2;
		numitems = ze.Index;

		// Unzip each item, using the name stored (in the zip) for that item.
		for (ze.Index = 0; ze.Index < numitems; ze.Index++)
		{				
			// Get info about the next item
			if ((result = lpUnzipGetItem(huz, &ze))) goto bad2;
			{
				unsigned char	*buffer;

				// Allocate a memory buffer to decompress the item
				if (!(buffer = (unsigned char*)GlobalAlloc(GMEM_FIXED, ze.UncompressedSize)))
				{
					show_errmsg();
					break;
				}

				// Decompress the item into our buffer
				if ((result = lpUnzipItemToBuffer(huz, buffer, ze.UncompressedSize, &ze)))
				{
					GlobalFree(buffer);
bad2:					
					lpUnzipClose(huz);
					goto bad;
				}

				// Here we would do something with the contents of buffer. It contains
				// ze.UncompressedSize bytes.
				//이 부분에다가 매니저 클래스를 만들어야 한다.
				GlobalFree(buffer);
			}
		}		

		bResult = true;
		// Done unzipping files, so close the ZIP archive.
		lpUnzipClose(huz);
	}
	else
	{
		TCHAR	msg[100];
bad:		
		lpUnzipFormatMessage(result, msg, sizeof(msg));
		OutputDebugString(msg);
	}

	return bResult;
}

unsigned char* SFFilePack::UnPackResMem( TCHAR* pExtractedFileName, int& BufferSize )
{
	BufferSize = 0;

	HUNZIP		huz;
	DWORD		result;
	unsigned char     *buffer = NULL;

	// Load/Open the ZIP archive, which is in our EXE's resources. It is an RT_RCDATA type
	// of resource with an ID number of 1.
	if (!(result = lpUnzipOpenBuffer(&huz, 0, 1, 0)))
	{
		ZIPENTRY		ze;

		lstrcpy(ze.Name, pExtractedFileName);

		result = lpUnzipFindItem(huz, &ze, 0);

		if(!result)
		{
			buffer = (unsigned char *)GlobalAlloc(GMEM_FIXED, ze.UncompressedSize);

			if(!buffer)
			{
				show_errmsg();
			}
			else
			{
				if(result = lpUnzipItemToBuffer(huz, buffer, ze.UncompressedSize, &ze))
				{
					GlobalFree(buffer);
					buffer = NULL;
				}
				else
				{						
					BufferSize = ze.UncompressedSize;
				}					
			}				
		}

		lpUnzipClose(huz);
	}

	return buffer;
}

unsigned char* SFFilePack::UnPackMemoryRaw( TCHAR* pZipFileName, unsigned char* pBuffer, int BufferSize )
{
	UNREFERENCED_PARAMETER(pBuffer);
	UNREFERENCED_PARAMETER(BufferSize);

	HUNZIP		huz;
	DWORD		result;		
	unsigned char	*buffer = NULL;

	// Open a ZIP archive on disk named "test.zip".
	if (!(result = lpUnzipOpenFileRaw(&huz, pZipFileName, 0)))
	{
		ZIPENTRY		ze;


		// Because the zip archive was created "raw" (ie, without any ZIP
		// header), then we MUST know what the original size is, as well as
		// the compressed size. We put these two values in the ZIPENTRY before
		// we call one of the UnzipItemXXX functions
		ze.CompressedSize = 67;
		ze.UncompressedSize = 69;

		// Allocate a memory buffer to decompress the item
		if (!(buffer = (unsigned char *)GlobalAlloc(GMEM_FIXED, ze.UncompressedSize)))
		{
			TCHAR msg[160];

			msg[0] = 0;
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), &msg[0], 160, 0);
			OutputDebugString(msg);
		}
		else
		{
			// Decompress the item into our buffer
			if ((result = lpUnzipItemToBuffer(huz, buffer, ze.UncompressedSize, &ze)))
			{
				GlobalFree(buffer);
				buffer = NULL;

				lpUnzipClose(huz);
				goto bad;
			}

			// Here we would do something with the contents of buffer. It contains
			// ze.UncompressedSize bytes. We'll just display it to the console.
			//WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), buffer, ze.UncompressedSize, &ze.CompressedSize, 0);

			// We have no further use for the buffer, so we must free it.
			//GlobalFree(buffer);
		}

		// Done unzipping files, so close the ZIP archive.
		lpUnzipClose(huz);
	}
	else
	{
		TCHAR	msg[100];

bad:	
		lpUnzipFormatMessage(result, msg, sizeof(msg));
		OutputDebugString(msg);
	}

	return buffer;
}

