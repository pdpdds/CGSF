#pragma once
#include "ITest.h"

class TomCryptTest : public ITest
{
public:
	TomCryptTest(void);
	virtual ~TomCryptTest(void);
	
	virtual bool Run() override;

private:
	void HMACTest();
	bool R4Test();
};

