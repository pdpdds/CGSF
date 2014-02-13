#pragma once
#include "ITest.h"

class SFExcelTest : public ITest
{
public:
	SFExcelTest(void);
	virtual ~SFExcelTest(void);

	virtual bool Run() override;
};

