#pragma once
#include "ITest.h"

class CRCTest : public ITest
{
public:
	CRCTest(void);
	virtual ~CRCTest(void);

	virtual bool Run() override;

};

