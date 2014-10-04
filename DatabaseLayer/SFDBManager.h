#pragma once
#include <ACE/Method_Request.h>
#include <ACE/Task.h>
#include <ACE/OS.h>
#include <ACE/Condition_T.h>
#include "SFDBWorker.h"
#include "SFLockQueue.h"
#include "SFDBRequest.h"
#include "SFDataBaseProxy.h"
#include "IDBManager.h"

template <typename T>
class SFDBManager : public ACE_Task_Base, public IDBManager
{
public:
	enum
	{		
		MAX_TIMEOUT = 5,
	};

	SFDBManager()
		: m_ShutDown(0)
		, m_Workers_Lock()
		, m_workers_Cond(m_Workers_Lock)
	{		
	}

	virtual ~SFDBManager(void)
	{
	}
	
	int perform(ACE_Method_Request* pReq)
	{
		return this->m_queue.enqueue(pReq);
	}

	void SetDBParams(DBModuleParams& params)
	{
		m_DBparams = params;
	}

	int svc(void)
	{
		Create_Worker_Pool();

		while (m_ShutDown != 1)
		{
			ACE_Time_Value tv((long)MAX_TIMEOUT);
			tv += ACE_OS::time(0);

			SFDBRequest* pReq = (SFDBRequest*)this->m_queue.dequeue(&tv);

			if(NULL == pReq)
			continue;

			SFDBWorker* pWorker = ChooseDBWorker();
			
			pReq->SetOwner(pWorker);
			pWorker->perform(pReq);
		}

		SFDBWorker* pWorker = NULL;
		this->m_queueWorkers.dequeue_head(pWorker);

		while (pWorker)
		{						
			ACE_Thread_Manager::instance()->wait_task(pWorker);
			delete pWorker;
			pWorker = NULL;

			this->m_queueWorkers.dequeue_head(pWorker);
		}

		return 0;
	}

	int ShutDown()
	{
		m_ShutDown = 1;		
		
		return 0;
	}

	virtual int return_to_work(SFDBWorker* pWorker, ACE_Method_Request* pReq)
	{
		ACE_GUARD_RETURN(ACE_Thread_Mutex, worker_mon, this->m_Workers_Lock, -1);
		this->m_queueWorkers.enqueue_tail(pWorker);
		this->m_reqQueue.Push(pReq);
		this->m_workers_Cond.signal();

		return 0;
	}

	virtual int recall_request(ACE_Method_Request* pReq)
	{
		ACE_GUARD_RETURN(ACE_Thread_Mutex, worker_mon, this->m_Workers_Lock, -1);
		this->m_reqQueue.Push(pReq);

		return 0;
	}

	ACE_Method_Request* GetDBReq()
	{
		ACE_Method_Request* pReq = NULL;

		if(m_reqQueue.Size() == 0)
		{
			pReq = new SFDBRequest();
		}
		else
		{
			pReq = m_reqQueue.Pop();
		}

		return pReq;
	}

protected:
	SFDBWorker* ChooseDBWorker(void)
	{
		ACE_GUARD_RETURN(ACE_Thread_Mutex, worker_mon, this->m_Workers_Lock, 0);

		while(this->m_queueWorkers.is_empty())
			m_workers_Cond.wait();

		SFDBWorker* pWorker = NULL;
		this->m_queueWorkers.dequeue_head(pWorker);

		return pWorker;
	}

	int Create_Worker_Pool(void)
	{
		ACE_GUARD_RETURN(ACE_Thread_Mutex, worker_mon, this->m_Workers_Lock, -1);


		if (m_DBparams.threadSafe == true) //DB 모듈이 스레드 세이프하다면 스레드별로 별도의 객체를 가질 필요는 없다.
		{
			SFDatabase* pDatabase = new SFDatabase(new T());
			if (false == pDatabase->Initialize())
			{
				printf("SFDatabase Initialize Fail!!\n");
				return -1;
			}

			for (int i = 0; i < m_DBparams.workerPoolSize; i++)
			{
				SFDBWorker* pWorker;
				ACE_NEW_RETURN(pWorker, SFDBWorker(this), -1);
				this->m_queueWorkers.enqueue_tail(pWorker);

				pWorker->SetDatabase(pDatabase);

				pWorker->activate();
			}
		}
		else //DB 모듈이 스레드 세이프하지 않으므로 스레드마다 별도로 DB 모듈을 생성한다.
		{
			for (int i = 0; i < m_DBparams.workerPoolSize; i++)
			{
				SFDatabase* pDatabase = new SFDatabase(new T());
				if (false == pDatabase->Initialize())
				{
					printf("SFDatabase Initialize Fail!!\n");
					return -1;
				}

				SFDBWorker* pWorker;
				ACE_NEW_RETURN(pWorker, SFDBWorker(this), -1);
				this->m_queueWorkers.enqueue_tail(pWorker);

				pWorker->SetDatabase(pDatabase);

				pWorker->activate();
			}
		}

		return 0;
	}

	virtual bool done(void)
	{
		return (m_ShutDown == 1);
	}

	int Thread_ID(SFDBWorker* pWorker)
	{
		return pWorker->m_ThreadID;
	}

private:
	int m_ShutDown;
	DBModuleParams m_DBparams;
	ACE_Thread_Mutex m_Workers_Lock;
	ACE_Condition<ACE_Thread_Mutex> m_workers_Cond;
	ACE_Unbounded_Queue<SFDBWorker*>	m_queueWorkers;
	ACE_Activation_Queue	m_queue;
	SFLockQueue<ACE_Method_Request> m_reqQueue;
};
