#pragma once
#include "ITest.h"

class DumpTest : public ITest
{
public:
	DumpTest(void);
	virtual ~DumpTest(void);

	virtual bool Initialize() override;
	virtual bool Run() override;
	virtual bool Finally() override;

private:
	void GenerateCrash();
};

