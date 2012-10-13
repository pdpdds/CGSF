#include "StdAfx.h"
#include "SFEncryptionXOR.h"

SFEncryptionXOR::SFEncryptionXOR(void)
{
}

SFEncryptionXOR::~SFEncryptionXOR(void)
{
}

BOOL SFEncryptionXOR::Encrypt(BYTE* pSrc, DWORD Length)
{
	DWORD Remain = Length % sizeof(DWORD);
	Length -= Remain;

	for(DWORD i = 0; i < Length; i = i + sizeof(DWORD))
	{
		*(DWORD*)(pSrc + i) = *(DWORD*)(pSrc + i) ^ ENCRYPTION_KEY;
	}

	return TRUE;
}

BOOL SFEncryptionXOR::Decrypt(BYTE* pSrc, DWORD Length)
{
	DWORD Remain = Length % sizeof(DWORD);
	Length -= Remain;

	for(DWORD i = 0; i < Length; i = i + sizeof(DWORD))
	{
		*(DWORD*)(pSrc + i) = *(DWORD*)(pSrc + i) ^ ENCRYPTION_KEY;
	}

	return TRUE;
}
