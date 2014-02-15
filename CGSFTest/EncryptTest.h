#pragma once
#include "ITest.h"

class EncryptTest : public ITest
{
public:
	EncryptTest(void);
	virtual ~EncryptTest(void);

	virtual bool Run() override;
};
