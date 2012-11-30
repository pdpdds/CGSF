#pragma once

class SFCompressLzf
{
public:
	SFCompressLzf(void);
	virtual ~SFCompressLzf(void);

	BOOL Compress(BYTE* pDest, int& DestLen, BYTE* pSrc, int SrcLen);
	BOOL Uncompress(BYTE* pDest, int& DestLen, BYTE* pSrc, int SrcLen);
};