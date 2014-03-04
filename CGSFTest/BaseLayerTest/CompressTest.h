#pragma once
#include "ITest.h"

class CompressTest : public ITest
{
public:
	CompressTest(void);
	virtual ~CompressTest(void);

	virtual bool Run() override;

};

