#include "StdAfx.h"
#include "SFDBWorker.h"
#include "IDBManager.h"
#include "SFDatabase.h"
#include <ACE/TSS_t.h>
#include <ACE/OS.h>
#include "Macro.h"


SFDBWorker::SFDBWorker(IDBManager* pManager)
{ 
	m_pManager = pManager; 
}

SFDBWorker::~SFDBWorker(void)
{
	//if (m_pDatabase)
	//	delete m_pDatabase;
}

int SFDBWorker::svc(void)
{
	m_threadID = ACE_Thread::self();
	

	//BOOL bResult = m_pDatabase->Initialize();

	while (this->m_pManager->done() != true)
	{
		ACE_Time_Value Interval(5);
		Interval += ACE_OS::time(0);

		ACE_Method_Request* pReq = this->m_queue.dequeue(&Interval);
		
		if(NULL == pReq)
		{	
			continue;
		}

		int result = pReq->call();

		if(result == -1)
			break;

		this->m_pManager->return_to_work(this, pReq);
	}

	return 0;
}