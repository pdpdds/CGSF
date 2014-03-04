#pragma once
#include "SFBaseClass.h"

#define MAX_ARRAY_SIZE 1000000

class SFExtensionClass : public SFBaseClass
{
public:
	SFExtensionClass(void);
	virtual ~SFExtensionClass(void);

	int GetExtensionVar(){return m_iExtensionVar;}
	void SetExtensionVar(int iExtensionVar){m_iExtensionVar = iExtensionVar;}
	void InitArray();

	virtual int ProcessTask() override;
	int ProcessStackOverFlow();
	int ProcessStackOverFlow2();
	int ProcessHeapCorruption();
	void ProcessOutofMemory();


protected:

private:
	int m_iExtensionVar;
	int m_aArray[MAX_ARRAY_SIZE];
};
