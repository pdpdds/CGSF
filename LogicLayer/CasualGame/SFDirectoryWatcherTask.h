#pragma once
#include "SFShouter.h"

class SFDirectoryWatcherTask : public ACE_Task<ACE_SYNCH>
{
public:
	SFDirectoryWatcherTask(){m_done = FALSE; m_TaskProcessing = FALSE;}
	virtual ~SFDirectoryWatcherTask(void){}

	virtual int svc(void);

	BOOL GetTaskProcessing(){return m_TaskProcessing;}
	void SetTaskProcessing(BOOL TaskProcessing){m_TaskProcessing = TaskProcessing;}
	BOOL done(){return m_done;}

	HANDLE GetShouterEvent(){return m_hShouterEvent;}

protected:
	BOOL ProcessShouterTask();

private:
	SFShouter m_Shouter;
	HANDLE m_hShouterEvent;
	BOOL m_TaskProcessing;

	BOOL m_done;
};