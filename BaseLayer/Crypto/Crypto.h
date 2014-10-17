#pragma once
#include <wincrypt.h>
#include <iostream>

class CCrypto
{
public:
	CCrypto(void);
	virtual ~CCrypto(void);

	bool Initialize(CHAR* szKeyFile);
	bool ProcessEncryption(const TCHAR* pEncryptionData, int iDataSize);
	bool ProcessDecryption(const TCHAR* pDecryptionData, int iDataSize);

protected:
	bool Finally();

private:	
	HCRYPTPROV	m_hProv;
	HCRYPTKEY	m_hKey;
	HCRYPTKEY	m_hSessionKey;
	DWORD		m_cbBlob;
	BYTE*		m_pbBlob;
};
