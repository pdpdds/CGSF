#include "stdafx.h"
#include "RexTest.h"
#include "SFRegexChecker.h"

RexTest::RexTest(void)
{
}


RexTest::~RexTest(void)
{
}

bool RexTest::Run()
{
	SFRegexChecker Checker;

	//주민등록번호
	SFASSERT(FALSE == Checker.IsValidResidentRegistrationNumber(L"801234-798999"));
	SFASSERT(TRUE == Checker.IsValidResidentRegistrationNumber(L"801234-7989990"));
	SFASSERT(FALSE == Checker.IsValidResidentRegistrationNumber(L"801234-79899909"));

	//URL
	SFASSERT(TRUE == Checker.IsValidURL(L"http://onlinegameserver.org/444/"));
	SFASSERT(TRUE == Checker.IsValidURL(L"http://onlinegameserver.org/444"));
	SFASSERT(TRUE == Checker.IsValidURL(L"http://onlinegameserver.org"));
	SFASSERT(TRUE == Checker.IsValidURL(L"http://onlinegameserver"));
	SFASSERT(FALSE == Checker.IsValidURL(L"http://"));
	SFASSERT(FALSE == Checker.IsValidURL(L"http://["));

	//이메일
	SFASSERT(FALSE == Checker.IsValidEMail(L"email"));
	SFASSERT(FALSE == Checker.IsValidEMail(L"email@"));
	SFASSERT(FALSE == Checker.IsValidEMail(L"email@daum"));
	SFASSERT(FALSE == Checker.IsValidEMail(L"email@daum."));
	SFASSERT(TRUE == Checker.IsValidEMail(L"a@daum.net"));
	SFASSERT(TRUE == Checker.IsValidEMail(L"email-A@DAUM.Net"));
	SFASSERT(FALSE == Checker.IsValidEMail(L"email-A@DAUM.Neta"));
	SFASSERT(TRUE == Checker.IsValidEMail(L"email-A@DAUM.Ne"));
	SFASSERT(FALSE == Checker.IsValidEMail(L"email-A@DAUM.N"));
	SFASSERT(FALSE == Checker.IsValidEMail(L"email--A@DAUM.NET"));

	//IP Address
	SFASSERT(FALSE == Checker.IsValidIPAddress(L"333"));
	SFASSERT(FALSE == Checker.IsValidIPAddress(L"333."));
	SFASSERT(FALSE == Checker.IsValidIPAddress(L"333.333"));
	SFASSERT(FALSE == Checker.IsValidIPAddress(L"333.333."));
	SFASSERT(FALSE == Checker.IsValidIPAddress(L"333.333.333"));
	SFASSERT(FALSE == Checker.IsValidIPAddress(L"333.333.333."));
	SFASSERT(FALSE == Checker.IsValidIPAddress(L"333.333.333.333"));
	SFASSERT(TRUE == Checker.IsValidIPAddress(L"133.133.133.133"));
	SFASSERT(TRUE == Checker.IsValidIPAddress(L"255.255.255.255"));
	SFASSERT(FALSE == Checker.IsValidIPAddress(L"255.255.255.256"));
	SFASSERT(TRUE == Checker.IsValidIPAddress(L"56.255.255.255"));
	SFASSERT(FALSE == Checker.IsValidIPAddress(L"333.333.333.333."));
	SFASSERT(FALSE == Checker.IsValidIPAddress(L"333.333.333.333.333"));

	//캐릭터이름
	SFASSERT(FALSE == Checker.IsValidCharName(L"★가나다abc-][(){}_AB194"));
	SFASSERT(TRUE == Checker.IsValidCharName(L"가나다abc-][(){}_AB194"));
	SFASSERT(FALSE == Checker.IsValidCharName(L"ㅁ가나다abc-][(){}_AB194"));

	return true;
}
