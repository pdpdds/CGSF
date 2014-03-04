#include "stdafx.h"
#include "SFDirectoryWatcherTask.h"
#include "SFDirectoryWatcher.h"

int SFDirectoryWatcherTask::svc(void)
{
	m_hShouterEvent = CreateEvent(NULL, FALSE, FALSE, L"ShouterEvent");

	::setlocale(LC_ALL, "Korean");
	CDirectoryChangeWatcher			m_DirWatcher(false);
	SFDirectoryWatcher m_DirHandler(this);

	//DWORD dwChangeFilter = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_LAST_ACCESS | FILE_NOTIFY_CHANGE_SIZE | 
		//FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_LAST_ACCESS | FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_SECURITY;

	DWORD dwChangeFilter = FILE_NOTIFY_CHANGE_LAST_WRITE;
						 

	BOOL bWatchSubDir = TRUE;

	if( dwChangeFilter == 0 ){
		return 0;
	}


	//
	//	This demonstrates that the CDirectoryChangeHandler objects are 
	//	reference counted(reference count is incremented/decremented by CDirecotryChangeWatcher)
	//	and that they will be automatically deleted
	//	when the directory is unwatched.
	//

	TCHAR buffer[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, &buffer[0]);

	DWORD dwWatch;
	if( ERROR_SUCCESS != (dwWatch = m_DirWatcher.WatchDirectory(buffer, 
		dwChangeFilter,
		&m_DirHandler,
		bWatchSubDir,
		_T(""),
		_T(""))) )
	{
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
	while(done() == FALSE)
	{
		DWORD dwWait = WaitForSingleObject(m_hShouterEvent, 1000);

		if( dwWait == WAIT_OBJECT_0 )
		{
			ProcessShouterTask();

			m_TaskProcessing = FALSE;
		}
	}

	return 0;
}

BOOL SFDirectoryWatcherTask::ProcessShouterTask()
{
	m_Shouter.Initialize();
	m_Shouter.Read(_T("Shouter.xml"));
	_ShouterInfo* pInfo = m_Shouter.GetShouterInfo();

	if(pInfo->SentenceList.size() == 0)
		return FALSE;

	for(int i = 0; i < pInfo->RepeatCount; i++)
	{
		std::list<std::wstring>::iterator iter = pInfo->SentenceList.begin();


		if(i != 0)
			Sleep(pInfo->MessageInterval);

		DWORD Count = 0;

		for(;iter != pInfo->SentenceList.end(); iter++)
		{
			if(Count != 0)
				Sleep(pInfo->SentenceInterval);
 
			BasePacket* pCommand = PacketPoolSingleton::instance()->Alloc();
			pCommand->SetPacketType(SFPacket_Shouter);
			pCommand->SetOwnerSerial(-1);
			//memcpy(pCommand->GetDataBuffer(), iter._Ptr

			LogicGatewaySingleton::instance()->PushPacket(pCommand);

#ifdef _DEBUG
			_tprintf( _T("Shout : %s\n"), iter->c_str());
#endif

			Count++;
		}
	}

	return TRUE;
}