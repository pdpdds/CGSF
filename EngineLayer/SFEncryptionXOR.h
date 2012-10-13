#pragma once

class SFEncryptionXOR
{
public:
	SFEncryptionXOR(void);
	virtual ~SFEncryptionXOR(void);

	static BOOL Encrypt(BYTE* pSrc, DWORD Length);
	static BOOL Decrypt(BYTE* pSrc, DWORD Length);
};
