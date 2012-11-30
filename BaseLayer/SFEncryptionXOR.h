#pragma once

class SFEncryptionXOR
{
public:
	SFEncryptionXOR();
	virtual ~SFEncryptionXOR();

	static BOOL Encrypt(BYTE* pSrc, DWORD Length);
	static BOOL Decrypt(BYTE* pSrc, DWORD Length);

protected:

private:
	static DWORD m_EncrytionKey;
};
