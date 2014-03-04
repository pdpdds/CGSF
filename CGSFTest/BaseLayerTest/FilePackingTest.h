#pragma once
#include "ITest.h"

class FilePackingTest : public ITest
{
public:
	FilePackingTest(void);
	virtual ~FilePackingTest(void);

	virtual bool Run() override;
};

