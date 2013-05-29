// BitSet.h: interface for the CBitSet class.
//
// BitSet Mamangment Class
// 2002-03-08
// by bro ( bro@jiran.com )
// (Cho,Kyung-min)
//
// History
// 2002-03-18 Set function bug fix 
// 2002-03-19 % operator optimize
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BITSET_H__9C854732_4ED9_4D5D_84AB_8AFD4A9AA915__INCLUDED_)
#define AFX_BITSET_H__9C854732_4ED9_4D5D_84AB_8AFD4A9AA915__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CBitSet  
{
	BOOL		m_bAutoDelete;				// Is will be deleted?
	char*		m_pBitSet;					// Bitset raw memory pointer
	DWORD		m_dwUsedBytes;				// Used bytes.
public:

	CBitSet();
	virtual ~CBitSet();

	inline static DWORD sCalcUsedBytes( DWORD dwBits, BOOL b32BitAlign = TRUE )
	{
		DWORD dwUsedBytes = dwBits>>3;
		if( dwBits%8 ) dwUsedBytes++;

		if( b32BitAlign )
		{
			//<TODO> if os is 64 bit , then maybe change code little.
			dwUsedBytes += (4 - (dwUsedBytes&3) );
			
		}
		return dwUsedBytes;
	}

	BOOL	Create( DWORD dwBits, BOOL bZeroInit, BOOL b32BitAlign = TRUE, char* pszSource = NULL, DWORD dwSourceLen = 0 );

	const DWORD	GetUsedBytes() const
	{
		return m_dwUsedBytes;
	}
	inline char Get( DWORD dwBit )
	{
		return m_pBitSet[dwBit>>3] & 1<<(dwBit&7);
	}
	inline void Set( DWORD dwBit, char bit )
	{
		bit?m_pBitSet[dwBit>>3]|=1<<(dwBit&7):
			m_pBitSet[dwBit>>3]&=~(1<<(dwBit&7));		
	}

	inline char operator [] (int nBit)
	{
		return Get( nBit );
	}
};

#endif // !defined(AFX_BITSET_H__9C854732_4ED9_4D5D_84AB_8AFD4A9AA915__INCLUDED_)
