#include "stdafx.h"
#include "StringTest.h"
#include "SFString.h"
#include "SFFString.h"

#define MAX_NICKNAME_LEN 8
DECLARE_STRING_TYPE(SSNickName, MAX_NICKNAME_LEN);

StringTest::StringTest(void)
{
}


StringTest::~StringTest(void)
{
}


bool StringTest::Run()
{
	SSNickName szName;
	szName = _T("박주항");

	wprintf(L"%s\n", (TCHAR*)szName.GetBuffer());

	SFFString String("박주항");
	String.convert(SFFSTRType_Unicode16);
	wprintf(L"%s\n", String.getUnicode16());

	return true;
}