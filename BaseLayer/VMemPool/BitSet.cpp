// BitSet.cpp: implementation of the CBitSet class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BitSet.h"

/*#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif*/

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBitSet::CBitSet()
{
	m_pBitSet = NULL;
	m_dwUsedBytes = 0;
	m_bAutoDelete = FALSE;
}

CBitSet::~CBitSet()
{
	if( m_bAutoDelete )
	{
		if( m_pBitSet )
			delete [] m_pBitSet;
	}
}

//===========================================================
// Name   : Create
// Desc   : Initilize Bitset
// Param  : dwBits - bit count to use
//          bZeroInit - init to zero all bit
//          b32BitAlign - 4bytes alignment 
//          pszSource - provided memory block for using bltset
//          dwSourceLen - pszSource's length
// Return : if success , then return true
//----------------------------------------------------------
// Coder    Date              Desc
// bro      2002-03-08
//===========================================================
BOOL CBitSet::Create( DWORD dwBits , BOOL bZeroInit, BOOL b32BitAlign /*= TRUE*/, char* pszSource /*= NULL*/, DWORD dwSourceLen /*= 0 */)
{
	// Calculate bytes to use for bitset block
	m_dwUsedBytes = sCalcUsedBytes(dwBits, b32BitAlign);
	m_pBitSet = pszSource;
	
	// check block size
	if( pszSource && m_dwUsedBytes > dwSourceLen )
		return FALSE;

	// if source memory block isn't provided, then i'll use new memory block
	if( !pszSource ) 
	{
		m_pBitSet = new char[ m_dwUsedBytes ];
		// failed to create
		if(! m_pBitSet )
			return FALSE;
		m_bAutoDelete = TRUE;
	}

	if( bZeroInit )
		memset( m_pBitSet, 0, m_dwUsedBytes );

	return TRUE;
}