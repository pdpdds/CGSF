#include "stdafx.h"
#include "SFDirectoryWatcher.h"
#include "SFDirectoryWatcherTask.h"

SFDirectoryWatcher::SFDirectoryWatcher(SFDirectoryWatcherTask* pOwner)
	: m_pOwner(pOwner)
	, m_NextAcceptTick(0)
{
}


SFDirectoryWatcher::~SFDirectoryWatcher(void)
{
}

void SFDirectoryWatcher::On_FileModified(const CStdString & strFileName)
{
	if(m_pOwner->GetTaskProcessing() == TRUE)
		return;

	m_pOwner->SetTaskProcessing(TRUE);

	_tprintf( _T("File Modified: %s\n"),strFileName.c_str());
	
	if(NULL != wcsstr(strFileName.c_str(), _T("Shouter.xml")))
	{
		SetEvent(m_pOwner->GetShouterEvent());

	}
}