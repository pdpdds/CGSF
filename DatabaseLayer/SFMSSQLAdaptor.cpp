#include "stdafx.h"
#include "SFMSSQLAdaptor.h"
#include "SFMSSQL.h"

SFMSSQLAdaptor::SFMSSQLAdaptor(void)
	: m_pMSSql(NULL)
{
}


SFMSSQLAdaptor::~SFMSSQLAdaptor(void)
{		
	if(m_pMSSql)
		delete m_pMSSql;
}

BOOL SFMSSQLAdaptor::Initialize(_DBConnectionInfo* pInfo)
{
	m_pMSSql = new SFMSSQL();

	if(FALSE == m_pMSSql->Initialize(pInfo))
		return FALSE;
	
	return RegisterDBService();
}
