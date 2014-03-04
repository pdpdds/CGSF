#pragma once
#include "ITest.h"

class PCRETest : public ITest
{
public:
	PCRETest(void);
	virtual ~PCRETest(void);

	virtual bool Run() override;
};

