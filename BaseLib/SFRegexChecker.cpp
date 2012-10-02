#include "StdAfx.h"
#include "SFRegexChecker.h"
#include <regex>
#include <memory>
#include <string>

using namespace std;

SFRegexChecker::SFRegexChecker(void)
{
}

SFRegexChecker::~SFRegexChecker(void)
{
}

BOOL SFRegexChecker::IsValidCharName( TCHAR* szStr )
{

	if(NULL == szStr)
		return FALSE;

	tr1::wregex     regexCommandCapturePattern( L"(([°¡-ÆR]|[a-zA-Z0-9_]|[\\-\\[\\]\\(\\)\\{\\}])+)");
	tr1::wsmatch    regexMatchResult;

	std::wstring    wstrInputString(szStr );

	return regex_match( wstrInputString, regexMatchResult, regexCommandCapturePattern );
}

BOOL SFRegexChecker::IsValidResidentRegistrationNumber( TCHAR* szStr )
{
	if(NULL == szStr)
		return FALSE;

	tr1::wregex     regexCommandCapturePattern( L"\\d{6}\\-\\d{7}");
	tr1::wsmatch    regexMatchResult;

	std::wstring    wstrInputString(szStr );

	return regex_match( wstrInputString, regexMatchResult, regexCommandCapturePattern );
}

BOOL SFRegexChecker::IsValidURL( TCHAR* szStr )
{
	if(NULL == szStr)
		return FALSE;

	tr1::wregex     regexCommandCapturePattern( L"(ftp|http|https):\\/\\/(\\w+)(\\.\\w+)*(\\/([\\w\\d])+\\/{0,1})*");
	tr1::wsmatch    regexMatchResult;

	std::wstring    wstrInputString(szStr );

	return regex_match( wstrInputString, regexMatchResult, regexCommandCapturePattern );
}

BOOL SFRegexChecker::IsValidMacAddress( TCHAR* szStr )
{
	if(NULL == szStr)
		return FALSE;

	tr1::wregex     regexCommandCapturePattern( L"([0-9a-fA-F][0-9a-fA-F]-){5}([0-9a-fA-F][0-9a-fA-F])");
	tr1::wsmatch    regexMatchResult;

	std::wstring    wstrInputString(szStr );

	return regex_match( wstrInputString, regexMatchResult, regexCommandCapturePattern );
}

BOOL SFRegexChecker::IsValidEMail( TCHAR* szStr )
{
	if(NULL == szStr)
		return FALSE;

	tr1::wregex     regexCommandCapturePattern( L"[0-9a-zA-Z]([-_.]?[0-9a-zA-Z])*@[0-9a-zA-Z]([-_.]?[0-9a-zA-Z])*[.][a-zA-Z]{2,3}");
	tr1::wsmatch    regexMatchResult;

	std::wstring    wstrInputString(szStr );

	return regex_match( wstrInputString, regexMatchResult, regexCommandCapturePattern );
}

BOOL SFRegexChecker::IsValidIPAddress( TCHAR* szStr )
{
	if(NULL == szStr)
		return FALSE;

	tr1::wregex     regexCommandCapturePattern( L"([1]?\\d{1,2}|[2][0-4]\\d|25[0-5])[.]([1]?\\d{1,2}|[2][0-4]\\d|25[0-5])[.]([1]?\\d{1,2}|[2][0-4]\\d|25[0-5])[.]([1]?\\d{1,2}|[2][0-4]\\d|25[0-5])");

	tr1::wsmatch    regexMatchResult;

	std::wstring    wstrInputString(szStr );

	return regex_match( wstrInputString, regexMatchResult, regexCommandCapturePattern );
}