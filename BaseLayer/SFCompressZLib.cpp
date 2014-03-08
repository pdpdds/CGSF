#include "StdAfx.h"
#include "SFCompressZLib.h"
#include <zlib/zlib.h>

SFCompressZLib::SFCompressZLib(void)
{
}

SFCompressZLib::~SFCompressZLib(void)
{
}

bool SFCompressZLib::Compress(BYTE* pDest, int& DestLen, BYTE* pSrc, int SrcLen)
{
	int Result = compress(pDest,(uLongf*)&DestLen, pSrc, SrcLen);

	return Result >= 0;
}

bool SFCompressZLib::Uncompress(BYTE* pDest, int& DestLen, BYTE* pSrc, int SrcLen)
{
	int Result = uncompress(pDest, (uLongf*)&DestLen, pSrc, SrcLen);

	return Result >= 0;
}