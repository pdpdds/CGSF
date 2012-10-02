#pragma once
#include <ace/task_t.h>
#include <ace/Activation_Queue.h>
#include <ace/Method_Request.h>
#include "SFDatabaseMySQL.h"

class IDBManager;
extern ACE_TSS<SFDatabaseMySQL> Database;

class SFDBWorker : public ACE_Task<ACE_SYNCH>
{
	friend class SFDBManager;
public:
	SFDBWorker(IDBManager* pManager);
	virtual ~SFDBWorker(void);

	int perform(ACE_Method_Request* pReq)
	{
		return this->m_Queue.enqueue(pReq);
	}

	virtual int svc(void);

protected:

private:
	ACE_Activation_Queue m_Queue;
	ACE_thread_t m_ThreadID;
	IDBManager* m_pManager;
};

//#pragma comment(linker, "?DataBase@@3V?$ACE_TSS@VSFDatabaseMySQL@@@@A");
