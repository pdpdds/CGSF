#include "StdAfx.h"
#include "SFDBWorker.h"
#include "IDBManager.h"
#include "SFDatabase.h"
#include <ACE/TSS_t.h>
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
	ACE_Time_Value Interval(1, 1000);

	//BOOL bResult = m_pDatabase->Initialize();

	while (this->m_pManager->done() != true)
	{
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