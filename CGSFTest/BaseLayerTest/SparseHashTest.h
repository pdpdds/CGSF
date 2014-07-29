#pragma once
#include "ITest.h"

class SparseHashTest : public ITest
{
public:
	SparseHashTest();
	virtual ~SparseHashTest();

	virtual bool Run() override;
};

