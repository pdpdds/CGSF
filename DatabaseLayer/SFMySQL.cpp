#include "StdAfx.h"
#include "SFMySQL.h"
#include "StringConversion.h"

BOOL SFMySQL::Initialize(_DBConnectionInfo* pInfo)
{
	SFASSERT(mysql_thread_safe() > 0);

	LOG(INFO) << "MySQL Version : " <<  mysql_get_client_info();

	mysql_init(&m_Conn);

	m_pDBConnection = mysql_real_connect(&m_Conn, 
		StringConversion::ToASCII(pInfo->IP).c_str(),	
		StringConversion::ToASCII(pInfo->szUser).c_str(), 
		StringConversion::ToASCII(pInfo->szPassword).c_str(), 
		StringConversion::ToASCII(pInfo->szDBName).c_str(), 
		pInfo->Port,
		(char *)NULL, 
		0);

	if(m_pDBConnection==NULL)
	{
		LOG(INFO) << "Mysql Connection error";
		SFASSERT(0);
		return FALSE;
	}

	mysql_query(m_pDBConnection,"set session character_set_connection=euckr;");
	mysql_query(m_pDBConnection,"set session character_set_results=euckr;");
	mysql_query(m_pDBConnection,"set session character_set_client=euckr;");	

	return TRUE;
}

BOOL SFMySQL::Execute(char* szQuery)
{
	int QueryState;

	QueryState = mysql_query(m_pDBConnection, szQuery);

	if (QueryState != 0)
	{
		//LOG(FATAL) << "Mysql Query error" << mysql_error(&m_Conn);
		SFASSERT(0);
		return FALSE;
	}

	return TRUE;
}