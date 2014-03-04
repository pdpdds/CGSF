#pragma once
#include "ITest.h"

class ACELoggerTest : public ITest
{
public:
	ACELoggerTest(void);
	virtual ~ACELoggerTest(void);

	virtual bool Run() override;
};

