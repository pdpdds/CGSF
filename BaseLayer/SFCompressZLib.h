#pragma once

class SFCompressZLib
{
public:
	SFCompressZLib(void);
	virtual ~SFCompressZLib(void);

	BOOL Compress(BYTE* pDest, int& DestLen, BYTE* pSrc, int SrcLen);
	BOOL Uncompress(BYTE* pDest, int& DestLen, BYTE* pSrc, int SrcLen);

protected:

private:
};
