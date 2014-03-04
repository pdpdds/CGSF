#include "stdafx.h"
#include "RegistryTest.h"
#include "SFRegistry.h"

RegistryTest::RegistryTest(void)
{
}


RegistryTest::~RegistryTest(void)
{
}

bool RegistryTest::Run()
{

SFRegistry Reg(HKEY_LOCAL_MACHINE);

	if(TRUE == Reg.CreateRegistryKey(L"SOFTWARE\\JUHANG"))
	{
		DWORD dwValue;

		Reg.SetValue(L"TEST1", 555);
		Reg.GetValue(L"TEST1", dwValue);
		Reg.DeleteValue(L"TEST1");
		Reg.DeleteKey(L"SOFTWARE\\JUHANG");
	}

	return true;
}