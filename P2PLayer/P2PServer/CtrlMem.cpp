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
CCtrlMem::CCtrlMem()
{
	m_lMemSize = 0;
	m_lExtSize = CTRLMEM_DEFAULT_EXTSIZE;
	m_lCurrCharge = 0;
	m_pMem = NULL;
}

CCtrlMem::~CCtrlMem()
{
	if (m_pMem)
	{
		free(m_pMem);
		m_pMem = NULL;
	}
}

BOOL 
CCtrlMem::Init(LONG lSize, LONG lExtSize)
{
	m_pMem = malloc(lSize);
	if (NULL == m_pMem)
	{
		return FALSE;
	}

	m_lMemSize = lSize;
	m_lExtSize = lExtSize;
	m_lCurrCharge = 0;

	return TRUE;
}

void 
CCtrlMem::Uninit()
{
	m_lMemSize = 0;
	m_lCurrCharge = 0;

	if (m_pMem)
	{
		free(m_pMem);
		m_pMem = NULL;
	}
}

BOOL 
CCtrlMem::In(LONG lSize, const void* pMem)
{
	if (m_lMemSize < m_lCurrCharge + lSize)
	{
		void* pResizeMem = realloc(m_pMem, m_lCurrCharge + m_lExtSize + lSize);
		if (NULL == pResizeMem)
		{
			return FALSE;
		}

		m_lMemSize = m_lCurrCharge + m_lExtSize + lSize;
		m_pMem = pResizeMem;
	}

	memcpy((unsigned char*)m_pMem + m_lCurrCharge, pMem, lSize);
	m_lCurrCharge += lSize;

	return TRUE;
}

BOOL 
CCtrlMem::Out(LONG lSize, void* pMem)
{
	if (lSize > m_lCurrCharge)
	{
		return FALSE;
	}

	m_lCurrCharge -= lSize;

	memcpy(pMem, m_pMem, lSize);
	memmove(m_pMem, reinterpret_cast<unsigned char*>(m_pMem) + lSize, m_lCurrCharge);

	return TRUE;
}

BOOL 
CCtrlMem::Collection(LONG lSize)
{
	if (m_lCurrCharge == lSize)
	{
		m_lCurrCharge = 0;
		return TRUE;
	}

	if (m_lCurrCharge < lSize)
	{
		return FALSE;
	}

	m_lCurrCharge -= lSize;
	memmove(m_pMem, reinterpret_cast<unsigned char*>(m_pMem) + lSize, m_lCurrCharge);

	return TRUE;
}

BOOL 
CCtrlMem::Ext(LONG lSize)
{
	if (m_lMemSize < m_lCurrCharge + lSize)
	{
		void* pResizeMem = realloc(m_pMem, m_lCurrCharge + m_lExtSize + lSize);
		if (NULL == pResizeMem)
		{
			return FALSE;
		}

		m_lMemSize = m_lCurrCharge + m_lExtSize + lSize;
		m_pMem = pResizeMem;
	}

	return TRUE;
}

void
CCtrlMem::ChangeNoCharge()
{
	m_lCurrCharge = 0;
}

void 
CCtrlMem::Dig()
{
	m_lMemSize = 0;
	m_lExtSize = 0;
	m_lCurrCharge = 0;
	m_pMem = NULL;
}

