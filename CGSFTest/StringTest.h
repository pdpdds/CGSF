#pragma once
#include "ITest.h"

class StringTest : public ITest
{
public:
	StringTest(void);
	virtual ~StringTest(void);

	virtual bool Run() override;
};

