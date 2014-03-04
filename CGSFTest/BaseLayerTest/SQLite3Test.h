#pragma once
#include "ITest.h"

class SQLite3Test : public ITest
{
public:
	SQLite3Test(void);
	virtual ~SQLite3Test(void);

	virtual bool Run() override;
};

