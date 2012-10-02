#include "StdAfx.h"
#include "SFDBWorker.h"
#include "IDBManager.h"
#include "SFDatabaseMySQL.h"
#include <ACE/TSS_t.h>

ACE_TSS<SFDatabaseMySQL> Database;

SFDBWorker::SFDBWorker(IDBManager* pManager)
: m_pManager(pManager)
{
}

SFDBWorker::~SFDBWorker(void)
{
}

int SFDBWorker::svc(void)
{
	m_ThreadID = ACE_Thread::self();

	_DBConnectionInfo Info = {"cgsf", "test", "root", "root", "127.0.0.1", 3306};
	Database->Initialize(&Info);

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