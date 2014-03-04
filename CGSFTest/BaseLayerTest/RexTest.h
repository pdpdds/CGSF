#pragma once
#include "ITest.h"

class RexTest : public ITest
{
public:
	RexTest(void);
	virtual ~RexTest(void);

	virtual bool Run() override;
};

