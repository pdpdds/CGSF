#pragma once
#include "ITest.h"

class SFMathTest : public ITest
{
public:
	SFMathTest(void);
	virtual ~SFMathTest(void);

	virtual bool Run() override;
};

