#include "stdafx.h"
#include "CompressTest.h"
#include "SFCompressZLib.h"
#include "SFCompressLzf.h"
#include "SFCompressor.h"

#pragma comment(lib, "zlib.lib")
#pragma comment(lib, "liblzf.lib")

CompressTest::CompressTest(void)
{
}


CompressTest::~CompressTest(void)
{
}

#define MAX_DATA_SIZE 1024
bool CompressTest::Run()
{
	BYTE szSrc[MAX_DATA_SIZE] = "Casual Game Server Framework!!";
	BYTE szDest[MAX_DATA_SIZE] = {0,};
	int srcSize = strlen((char*)szSrc) + 1;
	int destSize = MAX_DATA_SIZE;

	SFCompressZLib* pCompressZLib = new SFCompressZLib();
	SFASSERT(TRUE == pCompressZLib->Compress(szDest, destSize, szSrc, srcSize));

	memset(szSrc, 0, MAX_DATA_SIZE);

	srcSize = destSize;
	destSize = MAX_DATA_SIZE;
	SFASSERT(TRUE == pCompressZLib->Uncompress(szSrc, destSize, szDest, srcSize));

	delete pCompressZLib;

	SFCompressLzf* pCompressLzf = new SFCompressLzf();

	srcSize = destSize;
	destSize = MAX_DATA_SIZE;
	SFASSERT(TRUE == pCompressLzf->Compress(szDest, destSize, szSrc, srcSize));

	memset(szSrc, 0, MAX_DATA_SIZE);

	srcSize = destSize;
	destSize = MAX_DATA_SIZE;
	SFASSERT(TRUE == pCompressLzf->Uncompress(szSrc, destSize, szDest, srcSize));

	delete pCompressLzf;

	return true;
}
