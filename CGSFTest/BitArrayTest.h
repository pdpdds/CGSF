#pragma once
#include "ITest.h"

class BitArrayTest : public ITest
{
public:
	BitArrayTest(void);
	virtual ~BitArrayTest(void);

	virtual bool Initialize() override;
	virtual bool Run() override;
	virtual bool Finally() override;
};

