#pragma once

class SFEncryptionXOR
{
public:
	SFEncryptionXOR();
	virtual ~SFEncryptionXOR();

	static bool Encrypt(BYTE* pSrc, DWORD Length);
	static bool Decrypt(BYTE* pSrc, DWORD Length);

protected:

private:
	static DWORD m_EncrytionKey;
};
