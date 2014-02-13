#include "stdafx.h"
#include "TomCryptTest.h"
//#include <tomcrypt.h>

//#pragma comment(lib, "tomcrypt.lib")

TomCryptTest::TomCryptTest(void)
{
}


TomCryptTest::~TomCryptTest(void)
{
}

bool TomCryptTest::Run()
{
	HMACTest();
	R4Test();

	return true;
}

void TomCryptTest::HMACTest()
{
	/*int idx, err; 
	hmac_state hmac; 
	unsigned char key[16] = "CGSF"; 
	unsigned char dst[MAXBLOCKSIZE]; 
	unsigned long dstlen; 

	if (register_hash(&sha1_desc) == -1) 
	{ 
		printf("Error registering SHA1\n"); 
		return ; 
	} 

	idx = find_hash("sha1"); 

	dstlen = sizeof(dst); 
	err = hmac_memory(idx, key, 4,(const unsigned char*)"hello", 5, dst, &dstlen); */
}

bool TomCryptTest::R4Test()
{
	/*prng_state prng; 
	unsigned char buf[32]; 

	int err; 
	if ((err = rc4_start(&prng)) != CRYPT_OK)
		return false; 
	
	if ((err = rc4_add_entropy((const unsigned char*)"CGSF", 4, &prng)) != CRYPT_OK) 
		return false; 

	if ((err = rc4_ready(&prng)) != CRYPT_OK) 
		return false; 

	strcpy((char*)buf,"hello world"); 
	if(rc4_read(buf, 11, &prng) != 11) 
		return false; 

	rc4_done(&prng); */

	return true;
}
