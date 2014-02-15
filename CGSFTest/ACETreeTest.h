#pragma once
#include "ITest.h"

class ACETreeTest : public ITest
{
public:
	ACETreeTest(void);
	virtual ~ACETreeTest(void);

	virtual bool Run() override;
};

