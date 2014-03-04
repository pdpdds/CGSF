#include "stdafx.h"
#include "SFMySQLAdaptor.h"
#include "SFMySQL.h"
#include "DBMsg.h"

SFMySQLAdaptor::SFMySQLAdaptor(void)
	: m_pMySql(NULL)
{
	
}


SFMySQLAdaptor::~SFMySQLAdaptor(void)
{
	if(m_pMySql)
		delete m_pMySql;
}

BOOL SFMySQLAdaptor::Initialize(_DBConnectionInfo* pInfo)
{
	m_pMySql = new SFMySQL();

	if(FALSE == m_pMySql->Initialize(pInfo))
		return FALSE;
	
	return RegisterDBService();
}