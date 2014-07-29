#include "StdAfx.h"
#include "Crypto.h"
#include "Key.h"
#include "tchar.h"
#include <malloc.h>

CCrypto::CCrypto(void)
: m_hProv(0)
, m_hKey(0)
, m_hSessionKey(0)
, m_cbBlob(0)
, m_pbBlob(NULL)
{

}

CCrypto::~CCrypto(void)
{
	Finally();
}

BOOL CCrypto::Initialize(CHAR* szKeyFile)
{			
	DWORD	dwResult;
	errno_t err;
	FILE* fp;

	m_cbBlob = file_length(szKeyFile);
	m_pbBlob = (BYTE*)malloc(m_cbBlob + 1);
	memset(m_pbBlob, 0, m_cbBlob + 1); 

	err = fopen_s(&fp, szKeyFile, "rb");
	if (err == 0) 
	{
		fread(m_pbBlob, 1, m_cbBlob, fp);
		fclose(fp);
	} 
	else 
	{
		free(m_pbBlob);
		m_pbBlob = NULL;
	}

	if (!CryptAcquireContext(&m_hProv, NULL, MS_DEF_PROV, PROV_RSA_FULL, 0))
	{
		dwResult = GetLastError();
		if (dwResult == NTE_BAD_KEYSET)
		{
			if (!CryptAcquireContext(&m_hProv, NULL, MS_DEF_PROV, PROV_RSA_FULL, CRYPT_NEWKEYSET))
			{
				dwResult = GetLastError();
				//MessageBox(_T("Error [0x%x]: CryptAcquireContext() failed."), _T("Information"), MB_OK);
				return FALSE;
			}
		} else {
			dwResult = GetLastError();
			return FALSE;
		}
	}

	if (m_pbBlob)
	{
		if (!CryptImportKey(m_hProv, m_pbBlob, m_cbBlob, 0, 0, &m_hSessionKey))
		{
			dwResult = GetLastError();
			//MessageBox(_T("Error [0x%x]: CryptImportKey() failed."), "Information", MB_OK);
			return FALSE;
		}
	} 
	else 
	{ 
		if (!CryptImportKey(m_hProv, PrivateKeyWithExponentOfOne, sizeof(PrivateKeyWithExponentOfOne), 0, 0, &m_hKey))
		{
			dwResult = GetLastError();
			//MessageBox(_T("Error CryptImportKey() failed."), _T("Information"), MB_OK);
			return FALSE;
		}

		if (!CryptGenKey(m_hProv, CALG_RC4, CRYPT_EXPORTABLE, &m_hSessionKey))
		{
			dwResult = GetLastError();
			//MessageBox(_T("Error CryptGenKey() failed."), _T("Information"), MB_OK);
			return FALSE;
		}	
	}

	return TRUE;
}

BOOL CCrypto::ProcessEncryption(const TCHAR* pEncryptionData, int iDataSize)
{		
	if(iDataSize <= 0)
		return FALSE;

	DWORD dwResult = 0;
	unsigned char * pCipherBlock = (unsigned char*)malloc(iDataSize);
	memset(pCipherBlock, 0, iDataSize);
	memcpy(pCipherBlock, pEncryptionData, iDataSize);

	DWORD length = iDataSize;

	if (!CryptEncrypt(m_hSessionKey, 0, TRUE, 0, pCipherBlock, &length, length))
	{
		dwResult = GetLastError();
		free(pCipherBlock);

		return FALSE;
	}

	memcpy((void*)pEncryptionData, pCipherBlock, length);

	free(pCipherBlock);

	return TRUE;	
}

BOOL CCrypto::ProcessDecryption(const TCHAR* pDecryptionData, int iDataSize)
{
	if(iDataSize <= 0)
		return FALSE;

	DWORD dwResult = 0;
	unsigned long length = iDataSize;
	unsigned char * pCipherBlock = (unsigned char*)malloc(length);
	memset(pCipherBlock, 0, length);
	memcpy(pCipherBlock, pDecryptionData, length);	

	if (!CryptDecrypt(m_hSessionKey, 0, TRUE, 0, pCipherBlock, &length))
	{
		dwResult = GetLastError();
		free(pCipherBlock);
		
		return FALSE;
	}

	memcpy((void*)pDecryptionData, pCipherBlock, length);	

	free(pCipherBlock);

	return TRUE;
}


BOOL CCrypto::Finally()
{
	if (!CryptReleaseContext(m_hProv,0))
	{
		return FALSE;
	}	

	return TRUE;
}
