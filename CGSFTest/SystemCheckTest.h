#pragma once
#include "ITest.h"

class SystemCheckTest : public ITest
{
public:
	SystemCheckTest(void);
	virtual ~SystemCheckTest(void);

	virtual bool Run() override;
};

