#include "stdafx.h"
#include "EncryptTest.h"
#include "SFEncryptionXOR.h"

EncryptTest::EncryptTest(void)
{
}


EncryptTest::~EncryptTest(void)
{
}

#define MAX_INPUT_SIZE 1024
bool EncryptTest::Run()
{
	BYTE szInputData[MAX_INPUT_SIZE] = "Casual Game Server Framework!!";

	SFEncryptionXOR Enc;
	Enc.Encrypt(szInputData, MAX_INPUT_SIZE);
	Enc.Decrypt(szInputData, MAX_INPUT_SIZE);

	return true;
}

