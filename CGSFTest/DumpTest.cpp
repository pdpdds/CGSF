#include "stdafx.h"
#include "DumpTest.h"
#include "SFExtensionClass.h"

#include "CrashHandler.h"
#include "SFBreakPad.h"
#include "SFCustomHandler.h"
#include "SFMinidump.h"
#include "SFBugtrap.h"

struct PureCallBase 
{ 
	PureCallBase() { mf(); } 
	void mf() 
	{ 
		pvf(); 
	} 
	virtual void pvf() = 0; 
};

struct PureCallExtend : public PureCallBase
{ 
	PureCallExtend() {}
	virtual void pvf() {}
};

void Func3()
{
	//int aLocalVar[2];
	//aLocalVar[3] = 0x45678;
}

void Func2()
{
	Func3();
}

void Func1()
{
	Func2();
}


DumpTest::DumpTest(void)
{
}


DumpTest::~DumpTest(void)
{
}

bool DumpTest::Initialize()
{
	return true;
}

bool DumpTest::Run()
{
	SFMinidump exceptionHandler;
	
	if(false == exceptionHandler.Install())
		return false;

	GenerateCrash();

	return true;
}

bool DumpTest::Finally()
{
	return true;
}

void DumpTest::GenerateCrash()
{
////////////////////////////////////////////////////////////////////////
//Generic Error
////////////////////////////////////////////////////////////////////////
	int* pPoint = 0;
	*pPoint = 1234;

////////////////////////////////////////////////////////////////////////
//CRT Error
////////////////////////////////////////////////////////////////////////
	/*TCHAR szData[100000] = L"SampleChatStringDataNoEndSpace!!";
	TCHAR szTargetBuffer[10] = {0,};
	_tcsncpy_s(szTargetBuffer, szData, 10);*/

////////////////////////////////////////////////////////////////////////
//Out Of Memory
////////////////////////////////////////////////////////////////////////
	/*SFExtensionClass* pExtensionClass = new SFExtensionClass();

	pExtensionClass->ProcessOutofMemory();*/

////////////////////////////////////////////////////////////////////////
//Heap Corruption
////////////////////////////////////////////////////////////////////////
	/*SFBaseClass* pBaseClass = new SFBaseClass();

	SFExtensionClass* pExtensionClass = static_cast<SFExtensionClass*>(pBaseClass);

	pExtensionClass->SetExtensionVar(12345678);
	pExtensionClass->ProcessHeapCorruption();	

	delete pBaseClass;*/

////////////////////////////////////////////////////////////////////////
//Pure Function Call
////////////////////////////////////////////////////////////////////////
	//PureCallExtend Temp;

////////////////////////////////////////////////////////////////////////
//Stack Overflow
////////////////////////////////////////////////////////////////////////
	/*SFExtensionClass* pExtensionClass = new SFExtensionClass();

	pExtensionClass->ProcessStackOverFlow();	

	delete pExtensionClass;*/

////////////////////////////////////////////////////////////////////////
//STATUS_ARRAY_BOUNDS_EXCEEDED : 확인해 볼 필요 있음
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
//Return Address Manipulation (Stack Destroy)
//2012에서는 해당 버그를 컴파일러 자체가 탐지하여 막아준다.
////////////////////////////////////////////////////////////////////////
	//Func1();
}