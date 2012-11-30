#pragma once
#include <ace/task_t.h>
#include <ace/Activation_Queue.h>
#include <ace/Method_Request.h>
#include "Macro.h"
#include "IDBManager.h"
#include "SFDatabase.h"

class SFDBWorker : public ACE_Task<ACE_SYNCH>
{
public:
	SFDBWorker(IDBManager* pManager){m_pManager = pManager;}
	virtual ~SFDBWorker(void){}

	int perform(ACE_Method_Request* pReq)
	{
		return this->m_Queue.enqueue(pReq);
	}

	virtual int svc(void)
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

	void SetDatabase(SFDatabase* pDatabase)
	{
		m_pDatabase = pDatabase;
	}
	SFDatabase* GetDatabase(){return m_pDatabase;}
		 
protected:

private:
	ACE_Activation_Queue m_Queue;
	ACE_thread_t m_ThreadID;
	IDBManager* m_pManager;
	SFDatabase* m_pDatabase;
};


//#pragma comment(linker, "?DataBase@@3V?$ACE_TSS@VSFDatabaseMySQL@@@@A");
