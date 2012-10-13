#include "StdAfx.h"
#include "SFDatabaseMySQL.h"
#include "SFDBRequest.h"
#include "SFMessage.h"
#include "DBMsg.h"

#pragma comment(lib, "libmySQL.lib")

SFDatabaseMySQL::SFDatabaseMySQL(void)
: m_pDBConnection(NULL)
{
	m_Dispatch.RegisterMessage(DBMSG_LOGIN, std::tr1::bind(&SFDatabaseMySQL::OnLogin, this, std::tr1::placeholders::_1));
}

SFDatabaseMySQL::~SFDatabaseMySQL(void)
{
	if(m_pDBConnection)
		mysql_close(m_pDBConnection);
}

BOOL SFDatabaseMySQL::Initialize(_DBConnectionInfo* pInfo)
{
	SFASSERT(mysql_thread_safe() > 0);

	//LOG(ERROR) << "MySQL Version : " <<  mysql_get_client_info();

	mysql_init(&m_Conn);

	m_pDBConnection = mysql_real_connect(&m_Conn, 
										 pInfo->IP,	
										 pInfo->szID, 
										 pInfo->szPassword, 
										 pInfo->szDBName, 
										 pInfo->Port,
										 (char *)NULL, 
										 0);
	
	if(m_pDBConnection==NULL)
	{
		//LOG(FATAL) << "Mysql Connection error";
		SFASSERT(0);
		return FALSE;
	}

	mysql_query(m_pDBConnection,"set session character_set_connection=euckr;");
	mysql_query(m_pDBConnection,"set session character_set_results=euckr;");
	mysql_query(m_pDBConnection,"set session character_set_client=euckr;");	

	return TRUE;
}

BOOL SFDatabaseMySQL::Execute(char* szQuery)
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

BOOL SFDatabaseMySQL::Call( SFMessage* pMessage )
{
	return m_Dispatch.HandleMessage(pMessage->GetCommand(), pMessage);
	
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
BOOL SFDatabaseMySQL::OnLogin( SFMessage* pMessage )
{
	MYSQL_RES *sql_result = NULL;
	//	MYSQL_ROW sql_row;

	int Result = -1;
/*	SFPacketStore::Login PktLogin;
	protobuf::io::ArrayInputStream is(pMessage->GetData(), pMessage->GetDataSize());
	PktLogin.ParseFromZeroCopyStream(&is);

	std::string username = PktLogin.username();*/

	char szQuery[100];
	sprintf_s(szQuery, "SELECT * FROM tblLogin WHERE UserName = '%s'", "cgsf");

	if(TRUE == Execute(szQuery))
	{
		sql_result = mysql_store_result(GetDBConnection());

		if(sql_result->row_count == 1)
		{
			Result = 0;
		}

		//while((sql_row=mysql_fetch_row(sql_result))!=NULL)
		//{
		//	printf("%2s %2s\n",sql_row[0],sql_row[1]);
		//}


		mysql_free_result(sql_result);
	}


////////////////////////////////////////////////////////////////////////////////
//결과를 로직 쓰레드로 보내야 할 경우
////////////////////////////////////////////////////////////////////////////////
	SFMessage* pMsg = GetDBInitMessage(pMessage->GetCommand(), pMessage->GetOwnerSerial());
	*pMsg << Result;
	SendToLogic(pMsg);

	
	return TRUE;
}

