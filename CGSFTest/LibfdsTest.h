#pragma once
#include "ITest.h"

class LibfdsTest : public ITest
{
public:
	LibfdsTest(void);
	virtual ~LibfdsTest(void);

	virtual bool Run() override;
};

