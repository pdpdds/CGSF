#pragma once

class SFCompressZLib
{
public:
	SFCompressZLib(void);
	virtual ~SFCompressZLib(void);

	bool Compress(BYTE* pDest, int& DestLen, BYTE* pSrc, int SrcLen);
	bool Uncompress(BYTE* pDest, int& DestLen, BYTE* pSrc, int SrcLen);

protected:

private:
};
