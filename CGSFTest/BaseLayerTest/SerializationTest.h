#pragma once
#include "ITest.h"

class SerializationTest : public ITest
{
public:
	SerializationTest(void);
	virtual ~SerializationTest(void);
		
	virtual bool Run() override;
};

