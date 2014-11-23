#pragma once
#include "ITest.h"

class SchedulerTest : public ITest
{
public:
	SchedulerTest();
	virtual ~SchedulerTest();

	bool Run();
};

