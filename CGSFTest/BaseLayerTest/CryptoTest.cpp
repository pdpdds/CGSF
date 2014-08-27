#include "stdafx.h"
#include "CryptoTest.h"
#include "Crypto/Crypto.h"
#include "Crypto/md5Capi.h"
#include "External/crc.h"

CryptoTest::CryptoTest()
{
}


CryptoTest::~CryptoTest()
{
}

bool CryptoTest::Run()
{
	CCrypto Crypto;

	Crypto.Initialize("Key.txt");

	TCHAR szJuhang[] = _T("juhang");

	Crypto.ProcessEncryption(szJuhang, sizeof(szJuhang));
	Crypto.ProcessDecryption(szJuhang, sizeof(szJuhang));

	///////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////
	unsigned char  test[] = "123456789";

	/*
	* Print the check value for the selected CRC algorithm.
	*/
	printf("The check value for the %s standard is 0x%X\n", CRC_NAME, CHECK_VALUE);

	/*
	* Compute the CRC of the test message, slowly.
	*/
	printf("The crcSlow() of \"123456789\" is 0x%X\n", crcSlow(test, 9));

	/*
	* Compute the CRC of the test message, more efficiently.
	*/
	crcInit();
	printf("The crcFast() of \"123456789\" is 0x%X\n", crcFast(test, 9));

	///////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////
	Cmd5Capi md5Capi;


	std::wstring out, in;
	in = L"JUHANG";

	out = md5Capi.Digest(in);

	return true;

}
