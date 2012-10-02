#pragma once
#include <vector>

using namespace std;

#define GetBit(a, b)	((((BYTE*)a)[(b)>>3] >> ((b)&7)) & 1)
#define SetBit(a, b)	(((BYTE*)a)[(b)>>3] |= (1 << ((b)&7)))
#define ResetBit(a, b)	(((BYTE*)a)[(b)>>3] &= ~(1 << ((b)&7)))
#define XOrBit(a, b)	(((BYTE*)a)[(b)>>3] ^= (1 << ((b)&7)))

#define 	SEG_COUNT		10240

enum Operation
{
	OR		= 0x00000001,
	AND		= 0x00000002,
	XOR		= 0x00000004,
	COPY	= 0x00000008,
	SET		= 0x00000001,
	RESET	= 0x00000002,
	GET		= 0x00000004,
	EQUAL	= 0x00000001,
	GREATER = 0x00000002,
	LESS	= 0x00000004,
	NOTEQUAL= 0x00000008,
};

void *AllocPtr(int nSize);
void *ReAllocPtr(void *p, int nSize);
void FreePtr(void *p);

class SFBitArray
{
public:
	SFBitArray(bool bCompressed = false);
	SFBitArray(SFBitArray &src);
	SFBitArray(BYTE* pBuffer, int nLength, bool bCompressed = false);
	virtual ~SFBitArray();

public:
	BYTE *GetBuffer()	{	return m_pBuffer;	}
	// returns byte steam pointer
	BYTE *Detach();
	// returns byte steam pointer and free internal buffer
	void Attach(BYTE* pBuffer, int nLength);
	// attachs the object with the input buffer
	void Init(BYTE* pBuffer, int nLength);
	// initializes the object with the input buffer
	int GetLength()	{	return m_nLength;	}
	// returns byte steam length
	bool IsEmpty();
	// check if the bitarray is empty (no ones)
	bool IsRangeEmpty(int nStartBit, int nEndBit);
	void SetRange(int nStartBit, int nEndBit);
	// set all range bits to 1
	void ResetRange(int nStartBit, int nEndBit);
	// reset all range bits
	void ResetAt(SFBitArray *pBitArray);
	// reset all bits that have ones values at the input bitarray
	void XOrRange(int nStartBit, int nEndBit);
	// toggle bits values of the input range
	void CopyRange(const SFBitArray& src, int nStartBit, int nEndBit);
	// copy bits values from the input bitarray range
	inline bool GetAt(int nBit)
	{
		if(m_bCompressed)
			return GetAt(m_pBuffer, m_nLength, nBit);
		if(nBit >= m_nLength<<3)
			return false;
		return GetBit(m_pBuffer, nBit);
	}
	inline void SetAt(int nBit)
	{
		if(m_bCompressed)
		{
			SetAt(m_pBuffer, m_nLength, nBit);
			m_nAllocLength = m_nLength;
		}
		else
		{
			if(nBit >= m_nLength<<3)
				SetLength((nBit>>3)+1);
			SetBit(m_pBuffer, nBit);
		}
		SetModified();
	}
	inline void ResetAt(int nBit)
	{
		if(nBit < m_nLength<<3)
			ResetBit(m_pBuffer, nBit);
		SetModified();
	}
	inline void XOrAt(int nBit)
	{
		if(nBit >= m_nLength<<3)
			SetLength((nBit>>3)+1);
		XOrBit(m_pBuffer, nBit);
		SetModified();
	}
	inline void SetAll()
	{
		memset(m_pBuffer, 0xff, m_nLength);
		SetModified();
	}
	inline void ResetAll()
	{
		memset(m_pBuffer, 0, m_nLength);
		SetModified();
	}
	int GetCount();
	// returns number of ones in the bitarray
	int GetRangeCount(int nStartBit, int nEndBit);
	// returns number of ones in the range of the bitarray
	void SetLength(int nLength);
	// sets bitarray byte steam length
	void FreeBuffer();
	// frees internal buffer
	int GetIndexBit(int nIndex);
	// returns the bit number of the one index
	int GetBitIndex(int nBit);
	// returns the index of the one in the input bit number
	void Delete(int nStart, int nEnd);
	// deletes the input range from bits stream
	void Insert(int nStart, int nCount, bool bSet);
	// insert values in a specified start bit

	inline bool operator[](int nBit)
	{	return GetAt(nBit);	}
	void operator=(const SFBitArray& src);
	void operator|=(const SFBitArray& src);
	void operator&=(const SFBitArray& src);
	void operator^=(const SFBitArray& src);
	bool operator==(const SFBitArray& src);
	bool operator!=(const SFBitArray& src);
	bool operator&&(const SFBitArray& src);
	SFBitArray operator&(const SFBitArray& src);
	SFBitArray operator|(const SFBitArray& src);
	SFBitArray operator^(const SFBitArray& src);
	void Compress();
	void Decompress();

	static void Compress(BYTE *src, int nSrcLen, BYTE *&des, int &ndesLen);
	static void Decompress(BYTE *&src, int &nSrcLen, int nMaxLen = -1);
	static int DecompressLength(BYTE *src, int nSrcLen);
	static bool SetAt(BYTE *&src, int &nSrcLen, int nBit);
	static bool GetAt(BYTE *src, int nSrcLen, int nBit);
	inline static void Mem(Operation op, BYTE *src, int srcLen, BYTE *&des, int &desLen)
	{
		if(op == AND)
		{
			desLen = min(desLen, srcLen);
			for(int nByte = 0; nByte < desLen; nByte++)
				des[nByte] &= src[nByte];
			while(desLen && des[desLen-1] == 0)
				desLen--;
			return;
		}
		if(desLen < srcLen)
		{
			if(desLen == 0 || des == NULL)
				des = (BYTE*)AllocPtr(srcLen);
			else
				des = (BYTE*)ReAllocPtr(des, srcLen);
			desLen = srcLen;
		}
		if(op == COPY)
			memcpy(des, src, srcLen);
		else	if(op == OR)
			for(int nByte = 0; nByte < srcLen; nByte++)
				des[nByte] |= src[nByte];
		else	if(op == XOR)
			for(int nByte = 0; nByte < srcLen; nByte++)
				des[nByte] ^= src[nByte];
	}
	inline static void Mem(Operation op, SFBitArray &src, SFBitArray &des)
	{
		if(op == AND)
			des &= src;
		else	if(op == COPY)
			des = src;
		else	if(op == OR)
			des |= src;
		else	if(op == XOR)
			des ^= src;
	}
	inline static void BitsNot(BYTE *&p, int &nLength, int nNewBitsLen)
	{
		int nNewLen = (nNewBitsLen+7)/8;
		if(nNewLen > nLength)
		{
			if(nLength == 0)
				p = (BYTE*)AllocPtr(nNewLen);
			else
				p = (BYTE*)ReAllocPtr(p, nNewLen);
			nLength = nNewLen;
		}
		for(int nByte = 0; nByte < nNewLen; nByte++)
			p[nByte] = ~p[nByte];
		for(int nBit = nNewBitsLen; nBit < nNewLen*8; nBit++)
			ResetBit(p, nBit);
	}

	inline static int GetBitsValue(BYTE *p, int nStartBit, int nBitCount)
	{
		int nValue = 0;
		for(int nBit = nStartBit; nBitCount--; nBit++)
			if(GetBit(p, nBit))
				SetBit(&nValue, nBit-nStartBit);
		return nValue;
	}
	inline static void SetAt(BYTE *src, int nSrcStartBit, BYTE *des, int nDesStartBit, int nBitCount)
	{
		while(nBitCount--)
		{
			if(GetBit(src, nSrcStartBit))
				SetBit(des, nDesStartBit);
			else
				ResetBit(des, nDesStartBit);
			nSrcStartBit++, nDesStartBit++;
		}
	}
	inline bool IsModified()
	{
		return m_bModified;
	}
	inline void SetModified(bool bModified)
	{
		m_bModified = bModified;
	}
	inline bool IsCompressed()
	{
		return m_bCompressed;
	}
	inline void SetCompressed(bool bCompressed)
	{
		m_bCompressed = bCompressed;
	}
	void Invert(int nMaxBits);
	// inverts all bits values to the input bit number
	int GetHeadBit();
	// return first '1' bit number
	int GetTailBit();
	// return last '1' bit number
	int GetActualBit(int dwIndexBit);
	int Bmp2Array(int *&pBuffer, bool bAllocated = false);
	int Bmp2Array(vector<int> &nIntArray);
	void Append2Array(vector<int> &nIntArray);
	int Range2Array(int nStartBit, int nEndBit, vector<int> &nIntArray);

protected:
	void InitValues();
	void SetModified();
	void Index();

protected:
	BYTE* m_pBuffer;	// byte steam pointer
	int m_nLength, m_nAllocLength;
	int m_nCount;	// '1's count in the bits stream
	int *m_nIndexes, m_nIndexesCount;
	int m_nBitSeg;
	bool m_bModified;
	bool m_bCompressed;
};
