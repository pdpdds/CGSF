#pragma once
#include <wincrypt.h>
#include <iostream>

class CCrypto
{
public:
	CCrypto(void);
	virtual ~CCrypto(void);

	BOOL Initialize(CHAR* szKeyFile);
	BOOL ProcessEncryption(const TCHAR* pEncryptionData, int iDataSize);
	BOOL ProcessDecryption(const TCHAR* pDecryptionData, int iDataSize);

protected:
	BOOL Finally();

private:	
	HCRYPTPROV	m_hProv;
	HCRYPTKEY	m_hKey;
	HCRYPTKEY	m_hSessionKey;
	DWORD		m_cbBlob;
	BYTE*		m_pbBlob;
};
