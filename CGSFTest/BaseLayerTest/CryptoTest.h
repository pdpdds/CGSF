#pragma once
#include "ITest.h"

class CryptoTest : public ITest
{
public:
	CryptoTest();
	virtual ~CryptoTest();

	virtual bool Run() override;
};

