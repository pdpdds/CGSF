#include "StdAfx.h"
#include "SFBaseClass.h"
#include "SFExtensionClass.h"

SFExtensionClass::SFExtensionClass(void)
: m_iExtensionVar(0)
{
}

SFExtensionClass::~SFExtensionClass(void)
{
}

void SFExtensionClass::InitArray()
{
	memset(m_aArray, 0, MAX_ARRAY_SIZE * sizeof(int));
}

int SFExtensionClass::ProcessTask()
{
	DWORD dwTickCount = GetTickCount();		

	int iLocalExtensionVar = (int)dwTickCount;

	return iLocalExtensionVar;
}

int SFExtensionClass::ProcessStackOverFlow()
{
	DWORD dwTickCount = GetTickCount();	

	this->ProcessStackOverFlow2();


	int iLocalExtensionVar = (int)dwTickCount;

	return iLocalExtensionVar;
}

int SFExtensionClass::ProcessStackOverFlow2()
{
	DWORD dwTickCount = GetTickCount();	

	this->ProcessStackOverFlow();

	return (int)dwTickCount;
}

int SFExtensionClass::ProcessHeapCorruption()
{
	DWORD dwTickCount = GetTickCount();		

	int iLocalExtensionVar = (int)dwTickCount;

	memset(m_aArray, 0, MAX_ARRAY_SIZE* sizeof(int));

	return iLocalExtensionVar;
}

void SFExtensionClass::ProcessOutofMemory()
{
	while(1)
	{
		SFExtensionClass* pExtensionClass = new SFExtensionClass();
	}
}