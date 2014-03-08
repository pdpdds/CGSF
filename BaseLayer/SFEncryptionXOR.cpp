#include "StdAfx.h"
#include "SFEncryptionXOR.h"

#define ENCRYPTION_KEY 0xAFB7E3D9

DWORD SFEncryptionXOR::m_EncrytionKey = ENCRYPTION_KEY;

SFEncryptionXOR::SFEncryptionXOR()
{
	
}

SFEncryptionXOR::~SFEncryptionXOR(void)
{
}

bool SFEncryptionXOR::Encrypt(BYTE* pSrc, DWORD Length)
{
	DWORD Remain = Length % sizeof(DWORD);
	Length -= Remain;

	for(DWORD i = 0; i < Length; i = i + sizeof(DWORD))
	{
		*(DWORD*)(pSrc + i) = *(DWORD*)(pSrc + i) ^ m_EncrytionKey;
	}

	return true;
}

bool SFEncryptionXOR::Decrypt(BYTE* pSrc, DWORD Length)
{
	DWORD Remain = Length % sizeof(DWORD);
	Length -= Remain;

	for(DWORD i = 0; i < Length; i = i + sizeof(DWORD))
	{
		*(DWORD*)(pSrc + i) = *(DWORD*)(pSrc + i) ^ m_EncrytionKey;
	}

	return true;
}
