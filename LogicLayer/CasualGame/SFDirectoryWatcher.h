#pragma once
#include "DirectoryChanges.h"
#include "DelayedDirectoryChangeHandler.h"

class SFDirectoryWatcherTask;

class SFDirectoryWatcher : public CDirectoryChangeHandler
{
public:
	SFDirectoryWatcher(SFDirectoryWatcherTask* pOwner);
	virtual ~SFDirectoryWatcher(void);

	void On_FileModified(const CStdString & strFileName) override;

protected:

private:
	SFDirectoryWatcherTask* m_pOwner;
	DWORD m_NextAcceptTick;

};

