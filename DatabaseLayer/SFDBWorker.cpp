#include "StdAfx.h"
#include "SFDBWorker.h"
#include "IDBManager.h"
#include "SFDatabase.h"
#include <ACE/TSS_t.h>
#include "Macro.h"

int SFDBWorker::svc(void)
{
	m_ThreadID = ACE_Thread::self();

	//		BOOL bResult = m_pDatabase->Initialize();

	while(TRUE)
	{
		ACE_Method_Request* pReq = this->m_Queue.dequeue();

		if(NULL == pReq)
		{	
			SFASSERT(0);
			return -1;
		}

		int Result = pReq->call();

		if(Result == -1)
			break;

		this->m_pManager->return_to_work(this, pReq);
	}

	return 0;
}