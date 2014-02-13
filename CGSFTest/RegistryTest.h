#pragma once
#include "ITest.h"

class RegistryTest : public ITest
{
public:
	RegistryTest(void);
	virtual ~RegistryTest(void);

	virtual bool Run() override;
};

