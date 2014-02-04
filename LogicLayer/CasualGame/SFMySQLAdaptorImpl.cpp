#include "stdafx.h"
#include "SFMySQLAdaptorImpl.h"
#include "SFMySQL.h"
#include "DBMsg.h"
#include "SFSendDBRequest.h"

SFMySQLAdaptorImpl::SFMySQLAdaptorImpl(void)
	: m_pMySql(NULL)
{
	
}


SFMySQLAdaptorImpl::~SFMySQLAdaptorImpl(void)
{
	if(m_pMySql)
		delete m_pMySql;
}

BOOL SFMySQLAdaptorImpl::Initialize(_DBConnectionInfo* pInfo)
{
	m_pMySql = new SFMySQL();

	if(FALSE == m_pMySql->Initialize(pInfo))
		return FALSE;
	
	return RegisterDBService();
}


BOOL SFMySQLAdaptorImpl::RegisterDBService()
{
	m_Dispatch.RegisterMessage(DBMSG_LOGIN, std::tr1::bind(&SFMySQLAdaptorImpl::OnLogin, this, std::tr1::placeholders::_1));

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
BOOL SFMySQLAdaptorImpl::OnLogin( BasePacket* pPacket )
{
	SFMessage* pMessage = (SFMessage*)pPacket;
	MYSQL_RES *sql_result = NULL;
	//	MYSQL_ROW sql_row;

	int Result = -1;
	SFPacketStore::Login PktLogin;
	protobuf::io::ArrayInputStream is(pMessage->GetData(), pMessage->GetDataSize());
	PktLogin.ParseFromZeroCopyStream(&is);

	std::string username = PktLogin.username();

	char szQuery[100];
	sprintf_s(szQuery, "SELECT * FROM tblLogin WHERE UserName = '%s'", "cgsf");

	if(TRUE == m_pMySql->Execute(szQuery))
	{
		sql_result = mysql_store_result(m_pMySql->GetDBConnection());

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
	SFMessage* pMsg = SFDatabase::GetInitMessage(pMessage->GetCommand(), pMessage->GetOwnerSerial());
	*pMsg << Result;
	SFSendDBRequest::SendToLogic(pMsg);
	
	return TRUE;
}
