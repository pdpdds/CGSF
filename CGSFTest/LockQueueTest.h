#pragma once
#include "ITest.h"

class LockQueueTest : public ITest
{
public:
	LockQueueTest(void);
	virtual ~LockQueueTest(void);

	virtual bool Run() override;

private:
	bool TestLockQueue(ACE_THR_FUNC producer, ACE_THR_FUNC consumer, void* pArg);	

};

