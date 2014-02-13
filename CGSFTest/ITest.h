#pragma once
class ITest
{
public:
	ITest(void){}
	virtual ~ITest(void){}

	virtual bool Initialize() {return true;}
	virtual bool Run() = 0;
	virtual bool Finally() {return true;}
};

