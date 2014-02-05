#include "stdafx.h"
#include "SFFastDBAdaptor.h"


SFFastDBAdaptor::SFFastDBAdaptor(void)
	: m_pFastDB(NULL)
{
}


SFFastDBAdaptor::~SFFastDBAdaptor(void)
{
	if(m_pFastDB)
		delete m_pFastDB;
}

BOOL SFFastDBAdaptor::Initialize(_DBConnectionInfo* pInfo)
{
	m_pFastDB = new SFFastDB();

	if(FALSE == m_pFastDB->Initialize(pInfo->szDataSource))
		return FALSE;
	
	return RegisterDBService();
}