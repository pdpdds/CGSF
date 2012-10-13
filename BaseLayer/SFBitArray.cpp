#include "StdAfx.h"
#include "SFBitArray.h"

void *AllocPtr(int nSize)
{
	void *p = malloc(nSize);
	nSize = _msize(p);
	memset(p, 0, nSize);
	return p;
}

void *ReAllocPtr(void *p, int nSize)
{
	int nOldSize = _msize(p);
	p = realloc(p, nSize);
	if(nSize > nOldSize)
	{
		nSize = _msize(p);
		memset((char*)p+nOldSize, 0, nSize-nOldSize);
	}
	return p;
}

void FreePtr(void *p)
{
	free(p);
}

SFBitArray::SFBitArray(bool bCompressed /*= false*/)
{
	InitValues();
	m_bCompressed = bCompressed;
}

SFBitArray::SFBitArray(SFBitArray &src)
{
	InitValues();

	*this = src;
}

SFBitArray::SFBitArray(BYTE* pBuffer, int nLength, bool bCompressed /*= false*/)
{
	InitValues();

	m_bCompressed = bCompressed;
	Init(pBuffer, nLength);
}

void SFBitArray::InitValues()
{
	m_pBuffer = NULL;
	m_nLength = m_nAllocLength = 0;
	m_nCount = -1;
	m_nIndexes = NULL;
	m_bModified = false;
	m_nBitSeg = 1;
	m_bCompressed = false;
}

SFBitArray::~SFBitArray()
{
	if(m_pBuffer)
		FreePtr(m_pBuffer);
	if(m_nIndexes)
		free(m_nIndexes);
}

void SFBitArray::FreeBuffer()
{
	if(m_pBuffer)
		FreePtr(m_pBuffer);
	if(m_nIndexes)
		free(m_nIndexes);
	m_nIndexes = NULL;
	m_pBuffer = NULL;
	m_nLength = m_nAllocLength = 0;
	m_nCount = -1;
}

int SFBitArray::GetCount()
{
	if(m_nCount == -1)
	{	// (^_^) 19/2/2004 hmh
//		static BYTE bitCount[16] = { 0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4 };
		static BYTE bitCount[256] = { 
			0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4,1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
			1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
			1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
			2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
			1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
			2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
			2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
			3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,4,5,5,6,5,6,6,7,5,6,6,7,6,7,7,8 };
		BYTE by;
		m_nCount = 0;
		for(int nByte = 0; nByte < m_nLength; nByte++)
			if(by = m_pBuffer[nByte])
//				m_nCount += by == 0xff ? 8 : bitCount[by&0x0f] + bitCount[(by&0xf0) >> 4];
				m_nCount += bitCount[by];
	}
	return m_nCount;
}

int SFBitArray::GetRangeCount(int nStartBit, int nEndBit)
{	// (^_^) 6/3/2004 hmh
	static BYTE bitCount[16] = { 0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4 };

	int nCount = 0;
	BOOL bFirstByte = (nStartBit&7) != 0;
	if(bFirstByte)
		for(int nBit = nStartBit; nBit < (nStartBit/8+1)*8; nBit++)
			if(GetAt(nBit))
				nCount++;
	int nEndByte = nEndBit/8;
	BYTE by;
	for(int nByte = nStartBit/8+bFirstByte; nByte < nEndByte; nByte++)
		if(by = m_pBuffer[nByte])
			nCount += by == 0xff ? 8 : bitCount[by&0x0f] + bitCount[(by&0xf0) >> 4];
	for(int nBit = nEndByte*8; nBit <= nEndBit; nBit++)
		if(GetAt(nBit))
			nCount++;

	return nCount;
}

void SFBitArray::Index()
{	// (^_^) 21/2/2004 hmh
	if(GetLength() == 0)
		return;
	// calculate number of ones that will be include in each index
	m_nBitSeg = GetCount()/SEG_COUNT + 1;
	if(m_nIndexes)
		free(m_nIndexes);
	// allocate buffe of the indices array
	m_nIndexes = (int *)malloc(sizeof(int)*(m_nCount/m_nBitSeg+1));
	m_nIndexesCount = m_nCount = 0;
	BYTE by;
	// loop in the bitmap buffer to index '1's locations
	for(int nBit, nByte = 0; nByte < m_nLength; nByte++)
		// copy buffer byte into by and check if it is not 0
		if(by = m_pBuffer[nByte])
		{	// get bit number by multiply by 8 (or left shift by 3)
			nBit = nByte<<3;
			while(by)
			{	// if the first bit in the byte is '1'
				if(by&1)
					// check if the bit in the head of the index
					if(m_nCount++ % m_nBitSeg == 0)
						// add this bit to the indices
						m_nIndexes[m_nIndexesCount++] = nBit;
				// shift right to move second bit to the byte head
				by >>= 1, nBit++;
			}
		}
}

void SFBitArray::SetLength(int nLength)
{
	if(nLength == 0)
		FreeBuffer();
	else	if(nLength > m_nAllocLength)
	{
		m_nAllocLength = nLength+(m_bCompressed?0:100);
		if(m_pBuffer == NULL)
			m_pBuffer = (BYTE*)AllocPtr(m_nAllocLength);
		else
			m_pBuffer = (BYTE*)ReAllocPtr(m_pBuffer, m_nAllocLength);
	}
	m_nLength = nLength;
	m_bModified = true;
}

BYTE *SFBitArray::Detach()
{
	BYTE * p = m_pBuffer;
	m_pBuffer = NULL;
	FreeBuffer();
	
	return p;
}

void SFBitArray::Attach(BYTE* pBuffer, int nLength)
{
	FreeBuffer();
	if(nLength > 0)
	{
		m_pBuffer = pBuffer;
		m_nLength = m_nAllocLength = nLength;
	}
	m_bModified = true;
}

void SFBitArray::SetRange(int nStartBit, int nEndBit)
{
	if(nEndBit >= m_nLength*8)
		SetLength(nEndBit/8+1);
	for(int nBit = nStartBit; nBit <= nEndBit; nBit++)
		SetBit(m_pBuffer, nBit);
	SetModified();
}

void SFBitArray::ResetRange(int nStartBit, int nEndBit)
{
	if(nEndBit >= m_nLength*8)
		nEndBit = m_nLength*8-1;
	for(int nBit = nStartBit; nBit <= nEndBit; nBit++)
		ResetBit(m_pBuffer, nBit);
	SetModified();
}

void SFBitArray::ResetAt(SFBitArray *pBitArray)
{
	int nLength = min(m_nLength, pBitArray->GetLength())*8;
	for(int nBit = 0; nBit < nLength; nBit++)
		if(pBitArray->GetAt(nBit))
			ResetBit(m_pBuffer, nBit);
	SetModified();
}

void SFBitArray::XOrRange(int nStartBit, int nEndBit)
{
	if(nEndBit >= m_nLength*8)
		SetLength(nEndBit/8+1);
	for(int nBit = nStartBit; nBit <= nEndBit; nBit++)
		XOrBit(m_pBuffer, nBit);
	SetModified();
}

void SFBitArray::CopyRange(const SFBitArray& src, int nStartBit, int nEndBit)
{
	if(nStartBit >= src.m_nLength*8)
		return;
	nEndBit = min(nEndBit, src.m_nLength*8-1);

	if(nEndBit >= m_nLength*8)
		SetLength(nEndBit/8+1);

	BOOL bFirstByte = (nStartBit&7) != 0;
	int nStartByte = nStartBit/8+bFirstByte, nEndByte = max(nStartByte, nEndBit/8);
	if(bFirstByte)
		for(int nBit = nStartBit; nBit < nStartByte*8; nBit++)
			GetBit(src.m_pBuffer, nBit) ? SetBit(m_pBuffer, nBit) : ResetBit(m_pBuffer, nBit);
	if(nEndByte > nStartByte)
		memcpy(m_pBuffer+nStartByte, src.m_pBuffer+nStartByte, nEndByte-nStartByte);
	for(int nBit = nEndByte*8; nBit <= nEndBit; nBit++)
		GetBit(src.m_pBuffer, nBit) ? SetBit(m_pBuffer, nBit) : ResetBit(m_pBuffer, nBit);
	SetModified();
}

void SFBitArray::Compress()
{
	if(m_bCompressed || m_nLength == 0)
		return;
	m_bCompressed = true;

	int nLength = m_nLength;
	BYTE *p = new BYTE[m_nLength];
	memcpy(p, m_pBuffer, m_nLength);
	FreeBuffer();
	Compress(p, nLength, m_pBuffer, m_nLength);
	delete p;
}

void SFBitArray::Decompress()
{
	if(!m_bCompressed || m_nLength == 0)
		return;
	m_bCompressed = false;

	Decompress(m_pBuffer, m_nLength);
}

#define COMPRESS_TYPE	WORD//BYTE
#define COMPRESS_SIZE	2//1
#define COMPRESS_COUNT	65535//255

void SFBitArray::Compress(BYTE *src, int nSrcLen, BYTE *&des, int &nDesLen)
{
	nDesLen = 1;	// keep first byte for compression info
	while(nSrcLen && src[nSrcLen-1] == 0)
		nSrcLen--;
	if(nSrcLen == 0)
	{
		des = NULL;
		nDesLen = 0;
		return;
	}
	int nLength = nSrcLen;
	if(nLength)
		des = (BYTE*)AllocPtr(nLength);
	des[0] = 1;	// COMPRESS_TYPE	WORD
	int nByte = 0, nRunLength;
	BYTE byType;
	while(nByte < nSrcLen)
	{
		if(nDesLen+5 > nLength)
		{
			nLength += 1024;
			des = (BYTE*)ReAllocPtr(des, nLength);
		}
		byType = des[nDesLen++] = src[nByte++];
		if(byType == 0 || byType == 0xff)
		{
			nRunLength = 1;
			while(nRunLength < COMPRESS_COUNT && nByte < nSrcLen && src[nByte] == byType)
				nRunLength++, nByte++;
			*(COMPRESS_TYPE*)(des+nDesLen) = nRunLength;
			nDesLen += COMPRESS_SIZE;
		}
	}
}

void SFBitArray::Decompress(BYTE *&src, int &nSrcLen, int nMaxLen /*= -1*/)
{
	if(nSrcLen == 0)
		return;
	int nDesLen = 0;
	int nLength = nSrcLen, nRunLength;
	BYTE* des = (BYTE*)AllocPtr(nLength);
	int nByte = 1;	// first byte kept for comprerssion info
	BYTE byType;
	while(nByte < nSrcLen && (nMaxLen == -1 || nDesLen < nMaxLen))
		if(src[nByte] == 0 || src[nByte] == 0xff)
		{
			byType = src[nByte++];
			nRunLength = *(COMPRESS_TYPE*)(src+nByte);
			nByte += COMPRESS_SIZE;
			if(nDesLen+nRunLength+10 >= nLength)
			{
				nLength = nDesLen+nRunLength+1024;
				des = (BYTE*)ReAllocPtr(des, nLength);
			}
			if(byType)	
				memset(des+nDesLen, byType, nRunLength);
			nDesLen += nRunLength;
		}
		else
		{
			if(nDesLen+10 >= nLength)
			{
				nLength = nDesLen+1024;
				des = (BYTE*)ReAllocPtr(des, nLength);
			}
			des[nDesLen++] = src[nByte++];
		}
	FreePtr(src);
	nSrcLen = nDesLen; 
	src = (BYTE *)ReAllocPtr(des, nSrcLen);
}

bool SFBitArray::SetAt(BYTE *&src, int &nSrcLen, int nBit)
{
	int nDesLen = 0;
	int nDesByte = nBit/8;
	if(nSrcLen > 0)
	{
		int nByte = 1;	// first byte kept for comprerssion info
		int nRunLength = 0, nAddedSize;
		BYTE byType = 0;
		while(nByte < nSrcLen)
			if(src[nByte] == 0 || src[nByte] == 0xff)
			{
				byType = src[nByte++];
				nRunLength = *(COMPRESS_TYPE*)(src+nByte);
				if(nDesLen+nRunLength > nDesByte)
				{
					if(byType == 0xff)
						return false;
					// current buffer (0|count) and nByte points to count
					if(nRunLength > 1)
					{	//  (0|count-1|byte) OR (0|count1|byte|0|count2) OR (byte|0|count-1)
						nAddedSize = 1;
						if(nDesByte == nDesLen+nRunLength-1)
						{	// (0|count-1|byte) last byte
							// change the run length
							*(COMPRESS_TYPE*)(src+nByte) = nDesByte-nDesLen;
							// increment buffer index after the old run length
							nByte += COMPRESS_SIZE;
						}
						else	if(nDesByte > nDesLen)
						{	// (0|count1|byte|0|count2) middle byte
							nAddedSize += 1+COMPRESS_SIZE;
							// change the run length
							*(COMPRESS_TYPE*)(src+nByte) = nDesByte-nDesLen;
							// increment buffer index after the old run length
							nByte += COMPRESS_SIZE;
						}
						else	// (byte|0|count-1) frist byte
						{
							*(COMPRESS_TYPE*)(src+nByte) -= 1;
							nByte--;
						}
						// increment buffer size to have new byte(one) + Zero byte + reminder run length
						src = (BYTE*)ReAllocPtr(src, nSrcLen+nAddedSize);
						// move the buffer after the old run length bytes
						memmove(src+nByte+nAddedSize, src+nByte, nSrcLen-nByte);
						memset(src+nByte, 0, nAddedSize);
						// increment buffer size new byte(one) + Zero byte + reminder run length
						nSrcLen += nAddedSize;
						if(nAddedSize > 1)	// save the reminder run length
							*(COMPRESS_TYPE*)(src+nByte+2) = nRunLength-1-(nDesByte-nDesLen);
					}
					else
					{	// remove the zero count 
						memmove(src+nByte, src+nByte+COMPRESS_SIZE, nSrcLen-nByte-COMPRESS_SIZE);
						// decrement buffer size by COMPRESS_SIZE
						nSrcLen -= COMPRESS_SIZE;
						// reset buffer tail
						memset(src+nSrcLen, 0, COMPRESS_SIZE);
						// back to point to the zero byte
						nByte--;
					}
					// set the target bit
					SetBit(src+nByte, nBit&7);
					return true;
				}
				nDesLen += nRunLength;
				nByte += COMPRESS_SIZE;
			}
			else
			{
				if(nDesLen++ == nDesByte)
				{
					if(GetBit(src+nByte, nBit&7))
						return false;
					SetBit(src+nByte, nBit&7);
					if(src[nByte] == 0xff)
						if(nRunLength != COMPRESS_COUNT && byType == 0xff)
						{	// increment previous runlength only
							memmove(src+nByte, src+nByte+1, nSrcLen-nByte-1);
							src[--nSrcLen] = 0;
							*(COMPRESS_TYPE*)(src+nByte-COMPRESS_SIZE) = nRunLength+1;
						}
						else	if(nByte+2 < nSrcLen && src[nByte+1] == 0xff && *(COMPRESS_TYPE*)(src+nByte+2) != COMPRESS_COUNT)
						{	// increment next runlength only
							memmove(src+nByte, src+nByte+1, nSrcLen-nByte-1);
							src[--nSrcLen] = 0;
							*(COMPRESS_TYPE*)(src+nByte+2) += 1;
						}
						else
						{
							src = (BYTE*)ReAllocPtr(src, nSrcLen+COMPRESS_SIZE);
							memmove(src+nByte+1+COMPRESS_SIZE, src+nByte+1, nSrcLen-nByte-1);
							*(COMPRESS_TYPE*)(src+nByte+1) = 1;
							nSrcLen += COMPRESS_SIZE;
						}
					return true;
				}
				nByte++;
				byType = 0;
			}
	}
	bool bAlloc = nSrcLen == 0;
	int nLength = nDesByte-nDesLen;
 	int nRun = nLength/COMPRESS_COUNT+((nLength%COMPRESS_COUNT)?1:0);
	if(nRun > 0)
		nSrcLen += nRun*(1+COMPRESS_SIZE)+1;
	else
		nSrcLen++;
	if(bAlloc)
	{
		nSrcLen++;	// keep first byte for compression info
		src = (BYTE*)AllocPtr(nSrcLen);
		src[0] = 1;
	}
	else
		src = (BYTE*)ReAllocPtr(src, nSrcLen);
	while(nRun > 0)
		*(COMPRESS_TYPE*)(src+nSrcLen-(nRun--)*(1+COMPRESS_SIZE)) = (nLength>=COMPRESS_COUNT?COMPRESS_COUNT:nLength), nLength -= COMPRESS_COUNT;
	SetBit(src+nSrcLen-1, nBit&7);
	return true;
}

bool SFBitArray::GetAt(BYTE *src, int nSrcLen, int nBit)
{
	int nDesLen = 0, nDesByte = nBit/8;
	if(nSrcLen > 0)
	{
		int nByte = 1, nRunLength = 0;
		while(nByte < nSrcLen)
			if(src[nByte] == 0 || src[nByte] == 0xff)
			{
				nRunLength = *(COMPRESS_TYPE*)(src+nByte+1);
				if(nDesLen+nRunLength > nDesByte)
					return src[nByte] == 0xff;
				nDesLen += nRunLength;
				nByte += COMPRESS_SIZE+1;
			}
			else
			{
				if(nDesLen++ == nDesByte)
					return GetBit(src+nByte, nBit&7);
				nByte++;
			}
	}
	return false;
}

int SFBitArray::DecompressLength(BYTE *src, int nSrcLen)
{
	int nDesLen = 0, nByte = 0;
	while(nByte < nSrcLen)
		if(src[nByte] == 0 || src[nByte] == 0xff)
			nDesLen += *(COMPRESS_TYPE*)(src+(++nByte)), nByte += COMPRESS_SIZE;
		else
			nDesLen++, nByte++;
	return nDesLen;
}

int SFBitArray::GetIndexBit(int nIndex)
{
	if(nIndex <= -1 || nIndex >= GetCount())
		return -1;
	if(m_nIndexes == NULL)
		Index();

	int nMapIndex = nIndex / m_nBitSeg;
	if(nMapIndex >= m_nIndexesCount)
		return -1;
	if(m_nBitSeg == 1)
		return m_nIndexes[nIndex];
	int dwBitmapIndex = m_nIndexes[nMapIndex];
	int lReminder = nIndex % m_nBitSeg, nMaxBit = GetLength()<<3;
	while(lReminder > 0)
	{
		if(++dwBitmapIndex >= nMaxBit)
			return -1;
		if(GetAt(dwBitmapIndex))
			--lReminder;
	}
	return dwBitmapIndex;
}

int SFBitArray::GetBitIndex(int nBit)
{
	if(GetCount() == 0)
		return -1;
	if(m_nIndexes == NULL)
		Index();

	int nStart = 0, nEnd = m_nIndexesCount, nMapIndex = 0;
	while(nStart < nEnd)
	{
		nMapIndex = (nEnd+nStart)/2;
		if(nBit == m_nIndexes[nMapIndex])
			break;
		if(nBit < m_nIndexes[nMapIndex])
			nEnd = nMapIndex-1;
		else
			nStart = nMapIndex+1;
	}
	while(nMapIndex > 0 && nBit < m_nIndexes[nMapIndex])
		nMapIndex--;
	
	int nBitIndex = nMapIndex*m_nBitSeg;
	for(int dwCount = m_nIndexes[nMapIndex]; dwCount < nBit; dwCount++)
		if(GetAt(dwCount))
			nBitIndex++;
	if(GetAt(nBit) == FALSE)
		nBitIndex--;

	return nBitIndex;
}

void SFBitArray::Init(BYTE* pBuffer, int nLength)
{
	FreeBuffer();
	m_nLength = m_nAllocLength = nLength;
	if(nLength > 0)
	{
		m_pBuffer = (BYTE*)AllocPtr(nLength);
		memcpy(m_pBuffer, pBuffer, nLength);
		while(m_nLength  && m_pBuffer[m_nLength-1] == 0)
			m_nLength--;
	}
	m_bModified = true;
}

void SFBitArray::operator=(const SFBitArray& src)
{
	Init(src.m_pBuffer, src.m_nLength);
	m_nCount = src.m_nCount;
	if(src.m_nIndexes)
	{
		m_nIndexesCount = src.m_nIndexesCount;
		m_nIndexes = (int*)malloc(m_nIndexesCount*sizeof(DWORD));
		memcpy(m_nIndexes, src.m_nIndexes, m_nIndexesCount*sizeof(DWORD));
		m_nBitSeg = src.m_nBitSeg;
	}
	m_bModified = true;
}

void SFBitArray::operator|=(const SFBitArray& src)
{
	if(m_nLength < src.m_nLength)
		SetLength(src.m_nLength);
	for(int nByte = 0; nByte < src.m_nLength; nByte++)
		m_pBuffer[nByte] |= src.m_pBuffer[nByte];
	SetModified();
}

void SFBitArray::operator&=(const SFBitArray& src)
{
	m_nLength = min(m_nLength, src.m_nLength);
	for(int nByte = 0; nByte < m_nLength; nByte++)
		m_pBuffer[nByte] &= src.m_pBuffer[nByte];
	while(m_nLength  && m_pBuffer[m_nLength-1] == 0)
		m_nLength--;
	if(m_nLength == 0)
		FreeBuffer();
	SetModified();
}

void SFBitArray::operator^=(const SFBitArray& src)
{
	if(m_nLength < src.m_nLength)
		SetLength(src.m_nLength);
	for(int nByte = 0; nByte < src.m_nLength; nByte++)
		m_pBuffer[nByte] ^= src.m_pBuffer[nByte];
	SetModified();
}

bool SFBitArray::operator==(const SFBitArray& src)
{
	return m_nLength == src.m_nLength && memcmp(m_pBuffer, src.m_pBuffer, m_nLength) == 0;
}

bool SFBitArray::operator!=(const SFBitArray& src)
{
	return m_nLength != src.m_nLength || memcmp(m_pBuffer, src.m_pBuffer, m_nLength) != 0;
}

bool SFBitArray::operator&&(const SFBitArray& src)
{
	int nLength = min(m_nLength, src.m_nLength);
	for(int nByte = 0; nByte < nLength; nByte++)
		if((m_pBuffer[nByte] & src.m_pBuffer[nByte]) != 0)
			return true;
	return false;
}

SFBitArray SFBitArray::operator&(const SFBitArray& src)
{
	SFBitArray bitArray = *this;
	bitArray &= src;
	return bitArray;
}

SFBitArray SFBitArray::operator|(const SFBitArray& src)
{
	SFBitArray bitArray = *this;
	bitArray |= src;
	return bitArray;
}

SFBitArray SFBitArray::operator^(const SFBitArray& src)
{
	SFBitArray bitArray = *this;
	bitArray ^= src;
	return bitArray;
}

bool SFBitArray::IsEmpty()
{
	if(m_nLength == 0)
		return true;
	if(m_nCount > 0)
		return false;
	for(int nByte = 0; nByte < m_nLength; nByte++)
		if(m_pBuffer[nByte])
			return false;
	return true;
}

bool SFBitArray::IsRangeEmpty(int nStartBit, int nEndBit)
{
	if(m_nLength == 0)
		return true;
	if(nEndBit >= m_nLength*8)
		nEndBit = m_nLength*8-1;
	for(int dwBit = nStartBit; dwBit < (nStartBit/8+1)*8; dwBit++)
		if(GetAt(dwBit))
			return false;
	for(int nByte = nStartBit/8+1; nByte < nEndBit/8; nByte++)
		if(m_pBuffer[nByte])
			return false;
	if((nEndBit+1)&7)
		for(int dwBit = nEndBit/8*8; dwBit <= nEndBit; dwBit++)
			if(GetAt(dwBit))
				return false;
	return true;
}

void SFBitArray::Invert(int nMaxBits)
{
	SetModified();

	m_nLength = (nMaxBits+7)/8;
	if(m_nLength > m_nAllocLength)
	{
		m_nAllocLength = m_nLength;
		if(m_pBuffer == NULL)
			m_pBuffer = (BYTE*)AllocPtr(m_nAllocLength);
		else
			m_pBuffer = (BYTE*)ReAllocPtr(m_pBuffer, m_nAllocLength);
	}
	for(int nByte = 0; nByte < m_nLength; nByte++)
		m_pBuffer[nByte] = ~m_pBuffer[nByte];
	for(int nBit = nMaxBits; nBit < m_nLength*8; nBit++)
		ResetBit(m_pBuffer, nBit);
}

void SFBitArray::SetModified()
{
	m_nCount = -1;
	if(m_nIndexes)
		free(m_nIndexes);
	m_nIndexes = NULL;
	m_bModified = true;
}

void SFBitArray::Delete(int nStart, int nEnd)
{
	int nMaxBits = m_nLength*8;
	if(nStart >= nMaxBits || nStart > nEnd)
		return;
	nEnd = min(nEnd, nMaxBits-1);
	ResetRange(nStart, nEnd);
	for(int nIndex = nEnd+1; nIndex < nMaxBits; nIndex++)
		if(GetAt(nIndex))
		{
			ResetAt(nIndex);
			SetAt(nStart+nIndex-(nEnd+1));
		}
		else
			ResetAt(nStart+nIndex-(nEnd+1));
	m_bModified = true;
	nMaxBits = max(0, nMaxBits -(nEnd-nStart+1));
	m_nLength = nMaxBits/8 + ((nMaxBits%8)?1:0);
	for(int nIndex = nMaxBits; nIndex < nMaxBits+(8-nMaxBits%8); nIndex++)
		ResetBit(m_pBuffer, nIndex);
}

void SFBitArray::Insert(int nStart, int nCount, bool bSet)
{
	int nMaxBits = m_nLength*8;
	int lOldMaxBits = nMaxBits;
	SetLength(nMaxBits+nCount+max(0, nStart-nMaxBits));
	for(int nIndex = lOldMaxBits-1; nIndex > nStart; nIndex--)
		if(GetBit(m_pBuffer, nIndex))
		{
			ResetBit(m_pBuffer, nIndex);
			SetBit(m_pBuffer, nIndex+nCount);
		}
	if(bSet)
		SetRange(nStart+1, nStart+nCount);
	else
		ResetRange(nStart+1, nStart+nCount);
}

int SFBitArray::GetHeadBit()
{
	return GetIndexBit(0);
}

int SFBitArray::GetTailBit()
{
	return GetIndexBit(GetCount()-1);
}

int SFBitArray::GetActualBit(int dwIndexBit)
{
	if(GetAt(dwIndexBit))
		return dwIndexBit;
	int nIndex = GetBitIndex(dwIndexBit);
	if(nIndex == 0)
		return GetHeadBit();
	if(nIndex == GetCount())
		return GetTailBit();
	return GetIndexBit(nIndex);
}

int SFBitArray::Bmp2Array(int *&pBuffer, bool bAllocated /*= false*/)
{
	int nCount = GetCount();
	if(nCount == 0)
		return 0;
	if(bAllocated == false)
		pBuffer = (int*)AllocPtr(sizeof(int)*nCount);
	nCount = 0;
	BYTE by;
	for(int nBit, nByte = 0; nByte < m_nLength; nByte++)
		if(by = m_pBuffer[nByte])
		{
			nBit = nByte<<3;
			while(by)
			{
				if(by&1)
					pBuffer[nCount++] = nBit;
				by >>= 1, nBit++;
			}
		}
	return nCount;
}

int SFBitArray::Bmp2Array(vector<int> &nArray)
{
	nArray.resize(GetCount());
	return Bmp2Array((int*&)*nArray.begin(), true);
}

void SFBitArray::Append2Array(vector<int> &nArray)
{
	int nArraySize = nArray.size();
	nArray.resize(nArraySize+GetCount());
	BYTE by;
	for(int nBit, nByte = 0; nByte < m_nLength; nByte++)
		if(by = m_pBuffer[nByte])
		{
			nBit = nByte<<3;
			while(by)
			{
				if(by&1)
					nArray[nArraySize++] = nBit;
				by >>= 1, nBit++;
			}
		}
}

int SFBitArray::Range2Array(int nStartBit, int nEndBit, vector<int> &nArray)
{
	if(nStartBit >= m_nLength*8)
		return 0;
	nEndBit = min(nEndBit, m_nLength*8-1);

	int nCount = 0, nBit;
	BOOL bFirstByte = (nStartBit&7) != 0;
	if(bFirstByte)
		for(nBit = nStartBit; nBit < (nStartBit/8+1)*8; nBit++)
			if(GetAt(nBit))
				nArray.push_back(nBit), nCount++;
	int nEndByte = nEndBit/8;
	BYTE by;
	for(int nByte = nStartBit/8+bFirstByte; nByte < nEndByte; nByte++)
		if(by = m_pBuffer[nByte])
		{
			nBit = nByte<<3;
			while(by)
			{
				if(by&1)
					nArray.push_back(nBit), nCount++;
				by >>= 1, nBit++;
			}
		}
	for(nBit = nEndByte*8; nBit <= nEndBit; nBit++)
		if(GetAt(nBit))
			nArray.push_back(nBit), nCount++;
	return nCount;
}
