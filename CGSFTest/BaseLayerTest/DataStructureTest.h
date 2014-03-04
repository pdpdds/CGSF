#pragma once
#include "ITest.h"

class DataStructureTest : public ITest
{
public:
	DataStructureTest(void);
	virtual ~DataStructureTest(void);

	virtual bool Run() override;
};
