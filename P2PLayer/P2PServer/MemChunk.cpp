////////////////////////////////////////////////////////////////////////////////
//
// 작성자 : 이용휴
// 작성일 :
// 설  명 :
//

////////////////////////////////////////////////////////////////////////////////
//
#include "AllOcfCliSys.h"
#include "AllOcfCli.h"

////////////////////////////////////////////////////////////////////////////////
//
CMemChunk::CMemChunk()
{
	m_lMemSize = 0;
	m_lExtSize = MEMCHUNK_DEFAULT_EXTSIZE;
	m_pMem = NULL;
}

CMemChunk::~CMemChunk()
{
	if (m_pMem)
	{
		free(m_pMem);
		m_pMem = NULL;
	}
}

BOOL 
CMemChunk::Init(LONG lSize, LONG lExtSize)
{
	m_pMem = malloc(lSize);
	if (NULL == m_pMem)
	{
		return FALSE;
	}

	m_lMemSize = lSize;
	m_lExtSize = lExtSize;

	return TRUE;
}

void 
CMemChunk::Uninit()
{
	m_lMemSize = 0;

	if (m_pMem)
	{
		free(m_pMem);
		m_pMem = NULL;
	}
}

BOOL 
CMemChunk::Ext(LONG lSize)
{
	if (m_lMemSize < lSize)
	{
		void* pResizeMem = realloc(m_pMem, m_lExtSize + lSize);
		if (NULL == pResizeMem)
		{
			return FALSE;
		}

		m_lMemSize = m_lExtSize + lSize;
		m_pMem = pResizeMem;
	}

	return TRUE;
}


