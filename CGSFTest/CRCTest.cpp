#include "stdafx.h"
#include "CRCTest.h"
#include "SFFastCRC.h"

CRCTest::CRCTest(void)
{
}


CRCTest::~CRCTest(void)
{
}

bool CRCTest::Run()
{
	BYTE szCGSF[1024] = "Casual game server framework!!";
	SFFastCRC FastCrc;
	FastCrc.Initialize();
	DWORD crc;

	FastCrc.GetCRC(szCGSF, strlen((char*)szCGSF), crc);
	printf("The crcFast() of %s is 0x%X\n", szCGSF, crc);

	return true;
}
