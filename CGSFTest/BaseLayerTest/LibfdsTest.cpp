#include "stdafx.h"
#include "LibfdsTest.h"
#include "liblfds611.h"
#include "Mmsystem.h"

#pragma comment(lib, "Winmm.lib")

#pragma comment(lib, "liblfds611.lib")

static void PushThread(void* Args)
{
	

	struct lfds611_stack_state* pStack = (struct lfds611_stack_state*)Args;

	for(int i = 0; i < 3000; i++)
	{
		int* p = new int;
		*p = i;
		
		//stack_push(pStack, p);
		lfds611_stack_guaranteed_push(pStack, p);
		
	}
}

static void PopThread(void* Args)
{
	struct lfds611_stack_state* pStack = (struct lfds611_stack_state*)Args;

	while(1)
	{
		int* t = NULL;
		int i = lfds611_stack_pop(pStack, (void**)&t);

		if(t != NULL)
		{
			printf("pop success %d\n", GetCurrentThreadId());
			delete t;
		}
		else
		{
			//printf("pop fail\n");
		}
	}
}

LibfdsTest::LibfdsTest(void)
{
}


LibfdsTest::~LibfdsTest(void)
{
}

bool LibfdsTest::Run()
{
	struct lfds611_stack_state* pStack = NULL;
	lfds611_stack_new(&pStack, 10000);

	ACE::init();

	SYSTEM_INFO si;
	GetSystemInfo(&si);

	int OptimalThreadCount =si.dwNumberOfProcessors;

	DWORD dwTime = timeGetTime();
	int GroupID = ACE_Thread_Manager::instance()->spawn_n(OptimalThreadCount, (ACE_THR_FUNC)PushThread, pStack, THR_NEW_LWP, ACE_DEFAULT_THREAD_PRIORITY, 1);

	if(GroupID == -1)
	{
		SFASSERT(0);
	}

	if(ACE_Thread_Manager::instance()->spawn_n(OptimalThreadCount, (ACE_THR_FUNC)PopThread, pStack, THR_NEW_LWP, ACE_DEFAULT_THREAD_PRIORITY, 2) == -1)
	{
		SFASSERT(0);
	}

	ACE_Thread_Manager::instance()->wait_grp(GroupID);
	//ACE_Thread_Manager::instance()->wait_grp(Handler.grp_id());

	//Handler.wait();

	DWORD dwCompleteTime = timeGetTime();

	DWORD dwElapsed = dwCompleteTime - dwTime;

	printf("ElapsedTime %d\n", dwElapsed);

	ACE_Thread_Manager::instance()->wait_grp(2);

	ACE::fini();

	return true;
}
