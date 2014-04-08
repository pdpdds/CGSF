#include "stdafx.h"
#include "LockQueueTest.h"
#include "SFLockQueue.h"
#include "SFLockDeque.h"
#include "SFIOCPQueue.h"
#include "GPGLockFreeQueue.h"

volatile int releaseCount = 0;
int totalElementCount = 0;

#define MAX_ELEMENT_COUNT 100000
static void LockFreeProducer(void* Args)
{
	printf("producer spawn. thread id : %d\n", GetCurrentThreadId());

	GPG::LockFreeQueue<int>* pQueue = (GPG::LockFreeQueue<int>*)Args;

	for(int i = 0; i < MAX_ELEMENT_COUNT; i++)
	{
		
		GPG::node<int>* p = new GPG::node<int>(i);
		
		pQueue->Add(p);
	}
}

static void LockFreeConsumer(void* Args)
{
	printf("consumer spawn. thread id : %d\n", GetCurrentThreadId());

	GPG::LockFreeQueue<int>* pQueue = (GPG::LockFreeQueue<int>*)Args;

	while(releaseCount < totalElementCount)
	{
		GPG::node<int>* p = pQueue->Remove();

		if(p != NULL)
		{
			delete p;
			InterlockedIncrement((LONG *)&releaseCount);
		}
	}
}

static void IOCPProducerThread(void* Args)
{
	printf("producer spawn. thread id : %d\n", GetCurrentThreadId());

	SFIOCPQueue<int>* pQueue = (SFIOCPQueue<int>*)Args;

	for(int i = 0; i < MAX_ELEMENT_COUNT; i++)
	{
		int* p = new int;

		if(p == 0)
		{
			SFASSERT(0);
		}

		*p = i;
		pQueue->Push(p);
	}
}

static void IOCPConsumerThread(void* Args)
{
	printf("consumer spawn. thread id : %d\n", GetCurrentThreadId());

	SFIOCPQueue<int>* pQueue = (SFIOCPQueue<int>*)Args;
	while(releaseCount < totalElementCount)
	{
		int* pNum = pQueue->Pop(1);

		if(pNum)
		{		
			delete pNum;		
			InterlockedIncrement((LONG *)&releaseCount);
		}
	}
}

LockQueueTest::LockQueueTest(void)
{
}

LockQueueTest::~LockQueueTest(void)
{
}

bool LockQueueTest::Run()
{

//////////////////////////////////////////////////////////////////////////
//LockFreeQueueTest
//////////////////////////////////////////////////////////////////////////

	printf("phase 1. gpg lock queue test\n\n");

	GPG::node<int>* pNode = new GPG::node<int>(-1);
	GPG::LockFreeQueue<int> GPGQueue(pNode);
	VERIFY(true == TestLockQueue((ACE_THR_FUNC)LockFreeProducer, (ACE_THR_FUNC)LockFreeConsumer, &GPGQueue));
	SFASSERT(totalElementCount == releaseCount);
	
	printf("allocation count : %d, dealloc count : %d\n", totalElementCount, releaseCount);
//////////////////////////////////////////////////////////////////////////
//IOCP Lock Queue Test
//////////////////////////////////////////////////////////////////////////

	printf("\n\nphase 2. iocp lock queue test\n\n");

	//SFLockDeque<int> LockDeque;
	//SFLockQueue<int> LockQueue;
	SFIOCPQueue<int> IOCPQueue;
	
	totalElementCount = 0;
	releaseCount = 0;

	VERIFY(true == TestLockQueue((ACE_THR_FUNC)IOCPProducerThread, (ACE_THR_FUNC)IOCPConsumerThread, &IOCPQueue));
	SFASSERT(totalElementCount == releaseCount);

	printf("allocation count : %d, dealloc count : %d\n", totalElementCount, releaseCount);

	return true;
}

bool LockQueueTest::TestLockQueue(ACE_THR_FUNC producer, ACE_THR_FUNC consumer, void* pArg)
{
	ACE::init();

	int threadCount = 4;

	totalElementCount = threadCount * MAX_ELEMENT_COUNT;

	int producerGroupID = ACE_Thread_Manager::instance()->spawn_n(threadCount, producer, pArg, THR_NEW_LWP, ACE_DEFAULT_THREAD_PRIORITY, 1);

	SFASSERT(producerGroupID != -1);
	
	int consumerGroupID = ACE_Thread_Manager::instance()->spawn_n(threadCount, consumer, pArg, THR_NEW_LWP, ACE_DEFAULT_THREAD_PRIORITY, 2);

	SFASSERT(consumerGroupID != -1);

	ACE_Thread_Manager::instance()->wait_grp(producerGroupID);
	ACE_Thread_Manager::instance()->wait_grp(consumerGroupID);

	ACE::fini();

	return true;
}
