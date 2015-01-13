#include "stdafx.h"
#include "SFZdbAdpator.h"
#include "SFZdb.h"

SFZdbAdpator::SFZdbAdpator(void)
: m_pZdb(NULL)
{

}


SFZdbAdpator::~SFZdbAdpator(void)
{
	if (m_pZdb)
		delete m_pZdb;
}

BOOL SFZdbAdpator::Initialize(_DBConnectionInfo* pInfo)
{
	m_pZdb = new SFZdb();

	if (FALSE == m_pZdb->Initialize(pInfo))
		return FALSE;

	return RegisterDBService();
}