
//////////////////////////////////////////////////////////////////////
// md5Capi.cpp: implementation of the Cmd5Capi class.
// Calcule MD5 Digest using the WIN Crypto API.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "md5Capi.h"
#include "SFUtil.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Cmd5Capi::Cmd5Capi()
{
	csDigest.clear();
}

Cmd5Capi::Cmd5Capi(std::wstring & csBuffer)
{
	Digest(csBuffer);
}



Cmd5Capi::~Cmd5Capi()
{

}

std::wstring &Cmd5Capi::GetDigest(void)
{
	return csDigest; 

}


std::wstring &Cmd5Capi::Digest(std::wstring & csBuffer)
{
    HCRYPTPROV hCryptProv; 
    HCRYPTHASH hHash; 
    BYTE bHash[0x7f]; 
    DWORD dwHashLen= 16; // The MD5 algorithm always returns 16 bytes. 
	DWORD cbContent= csBuffer.length();
	BYTE* pbContent= (BYTE*)csBuffer.c_str();


    if(CryptAcquireContext(&hCryptProv, 
		NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_MACHINE_KEYSET)) 
	{

		if(CryptCreateHash(hCryptProv, 
			CALG_MD5,	// algorithm identifier definitions see: wincrypt.h
			0, 0, &hHash)) 
		{
			if(CryptHashData(hHash, pbContent, cbContent, 0))
			{

				if(CryptGetHashParam(hHash, HP_HASHVAL, bHash, &dwHashLen, 0)) 
				{
					// Make a string version of the numeric digest value
					csDigest.clear();
					
					  for (int i = 0; i<16; i++)
					  {
						  std::wstring tmp;						  
						  					//	tmp.append();

						  tmp = SFUtil::CustomFormatW(L"%02x", bHash[i]);
						
						
						  csDigest += tmp;
					  }

				}
				else csDigest=_T("Error getting hash param"); 

			}
			else csDigest=_T("Error hashing data"); 
		}
		else csDigest=_T("Error creating hash"); 

    }
    else csDigest=_T("Error acquiring context"); 


    CryptDestroyHash(hHash); 
    CryptReleaseContext(hCryptProv, 0); 	
    return csDigest; 


}
