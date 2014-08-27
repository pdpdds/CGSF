#include "stdafx.h"
#include "StringTest.h"
#include "SFString.h"
#include "SFFString.h"

#define MAX_NICKNAME_LEN 8
DECLARE_STRING_TYPE(SSNickName, MAX_NICKNAME_LEN); //선언부에 선언을 한다. 

StringTest::StringTest(void)
{
}


StringTest::~StringTest(void)
{
}


bool StringTest::Run()
{
	SSNickName szName;
	szName = _T("NickName");//MAX_NICKNAME_LEN보다 길다면 할당을 실패함.

	wprintf(L"%s\n", (TCHAR*)szName.GetBuffer());

	SFFString String("NickName");//멀티코드로 문자열 생성 
	String.convert(SFFSTRType_Unicode16); //유니코드로 변경

	wprintf(L"%s\n", String.getUnicode16());

	return true;
}
