#pragma once
#include <ACE/Method_Request.h>
#include <ACE/Task.h>
#include <ACE/OS.h>
#include <ACE/Condition_T.h>
#include "SFDBWorker.h"
#include "SFLockQueue.h"
#include "SFDBRequest.h"
#include "IDBManager.h"

template <typename T>
class SFDBManager : public ACE_Task_Base, public IDBManager
{
public:
	enum
	{
		DEFAUT_POOL_SIZE = 5,
		MAX_TIMEOUT = 5,
	};

	SFDBManager()
		: m_ShutDown(0)
		, m_Workers_Lock()
		, m_Workers_Cond(m_Workers_Lock)	
	{
		m_workerPoolSize = DEFAUT_POOL_SIZE;
	}

	virtual ~SFDBManager(void){}

	int perform(ACE_Method_Request* pReq)
	{
		return this->m_Queue.enqueue(pReq);
	}

	void SetWorkerPoolSize(int workerPoolSize)
	{
		m_workerPoolSize = workerPoolSize;
	}

	int svc(void)
	{
		Create_Worker_Pool();

		while(!done())
		{
			ACE_Time_Value tv((long)MAX_TIMEOUT);
			tv += ACE_OS::time(0);

			SFDBRequest* pReq = (SFDBRequest*)this->m_Queue.dequeue(&tv);

			if(NULL == pReq)
			continue;

			SFDBWorker* pWorker = ChooseDBWorker();
			
			pReq->SetOwner(pWorker);
			pWorker->perform(pReq);
		}

		return 0;
	}

	int ShutDown()
	{
		return 0;
	}

	virtual int return_to_work(SFDBWorker* pWorker, ACE_Method_Request* pReq)
	{
		ACE_GUARD_RETURN(ACE_Thread_Mutex, worker_mon, this->m_Workers_Lock, -1);
		this->m_queueWorkers.enqueue_tail(pWorker);
		this->m_ReqQueue.Push(pReq);
		this->m_Workers_Cond.signal();

		return 0;
	}

	virtual int recall_request(ACE_Method_Request* pReq)
	{
		ACE_GUARD_RETURN(ACE_Thread_Mutex, worker_mon, this->m_Workers_Lock, -1);
		this->m_ReqQueue.Push(pReq);

		return 0;
	}

	ACE_Method_Request* GetDBReq()
	{
		ACE_Method_Request* pReq = NULL;

		if(m_ReqQueue.Size() == 0)
		{
			pReq = new SFDBRequest();
		}
		else
		{
			pReq = m_ReqQueue.Pop();
		}

		return pReq;
	}

protected:
	SFDBWorker* ChooseDBWorker(void)
	{
		ACE_GUARD_RETURN(ACE_Thread_Mutex, worker_mon, this->m_Workers_Lock, 0);

		while(this->m_queueWorkers.is_empty())
			m_Workers_Cond.wait();

		SFDBWorker* pWorker = NULL;
		this->m_queueWorkers.dequeue_head(pWorker);

		return pWorker;
	}

	int Create_Worker_Pool(void)
	{
		ACE_GUARD_RETURN(ACE_Thread_Mutex, worker_mon, this->m_Workers_Lock, -1);

		for(int i = 0; i < m_workerPoolSize; i++)
		{
			SFDBWorker* pWorker;
			ACE_NEW_RETURN(pWorker, SFDBWorker(this), -1);
			this->m_queueWorkers.enqueue_tail(pWorker);

			SFDatabase* pDatabase = new SFDatabase(new T());
			//pDatabase->Initialize();

			pWorker->SetDatabase(pDatabase);

			pWorker->activate();
		}

		return 0;
	}

	int done(void)
	{
		return (m_ShutDown == 1);
	}

	int Thread_ID(SFDBWorker* pWorker)
	{
		return pWorker->m_ThreadID;
	}

private:
	int m_ShutDown;
	int m_workerPoolSize;
	ACE_Thread_Mutex m_Workers_Lock;
	ACE_Condition<ACE_Thread_Mutex> m_Workers_Cond;
	ACE_Unbounded_Queue<SFDBWorker*>	m_queueWorkers;
	ACE_Activation_Queue	m_Queue;
	SFLockQueue<ACE_Method_Request> m_ReqQueue;
};
