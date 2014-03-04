#pragma once
#include "ITest.h"

class GLogTest : public ITest
{
public:
	GLogTest(void);
	virtual ~GLogTest(void);

	virtual bool Run() override;
};