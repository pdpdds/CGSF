#pragma once
#include "ITest.h"

class SendEMailTest : public ITest
{
public:
	SendEMailTest(void);
	virtual ~SendEMailTest(void);
		
	virtual bool Run() override;
};

