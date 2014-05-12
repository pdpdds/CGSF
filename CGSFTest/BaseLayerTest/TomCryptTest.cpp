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
	//해쉬 알고리즘으로 SHA-1 등록 
	if (register_hash(&sha1_desc) == -1)
	{
		printf("Error registering SHA1\n");
		return -1;

		//Descriptor 테이블에서 SHA-1 해쉬 알고리즘의 인덱스 얻음 
		dx = find_hash("sha1");
		dstlen = sizeof(dst);
	//SHA-1 알고리즘과 키 "CGSF"를 이용하여 "hello"라는 데이터의 해싱 결과값을 dst에 얻음 
		err = hmac_memory(idx, key, 4, (const unsigned char*)"hello", 5, dst, &dstlen);*/
}

bool TomCryptTest::R4Test()
{
	/*prng_state prng; //의사 랜덤 넘버 제너레이터 
	unsigned char buf[32];

	int err;
	if ((err = rc4_start(&prng)) != CRYPT_OK) //RC4 초기화 
		return FALSE;
	//키 "CGSF"를 더해서 엔트로피를 증가시킴 
	if ((err = rc4_add_entropy((const unsigned char*)"CGSF", 4, &prng)) != CRYPT_OK)
		return FALSE;

	//RC4 준비 상태.. PRNG Init 
	if ((err = rc4_ready(&prng)) != CRYPT_OK)
		return FALSE;

	// 11바이트 "hello world" 버퍼 암호화 
	strcpy((char*)buf, "hello world");
	if (rc4_read(buf, 11, &prng) != 11)
		return FALSE;

	//암호화 결과 11바이트로 buf 버퍼에 저장이 됨 

	rc4_done(&prng);*/

	return true;
}
