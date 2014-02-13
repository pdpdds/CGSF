#pragma once
#include "ITest.h"

class INITest : public ITest
{
public:
	INITest(void);
	virtual ~INITest(void);

	virtual bool Initialize() override;
	virtual bool Run() override;
	virtual bool Finally() override;

};

