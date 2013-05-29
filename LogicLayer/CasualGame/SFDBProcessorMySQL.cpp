#include "stdafx.h"
#include "SFDBProcessorMySQL.h"
#include "DBMsg.h"
#include "SFSendDBRequest.h"

SFDBProcessorMySQL::SFDBProcessorMySQL(void)
{
	
}


SFDBProcessorMySQL::~SFDBProcessorMySQL(void)
{
}

BOOL SFDBProcessorMySQL::Initialize()
{
	_DBConnectionInfo Info = {"cgsf", "test", "juhang3", "genius", "127.0.0.1", 3306};
    SetInfo(Info);

	if(FALSE == SFDatabaseMySQL::Initialize())
		return FALSE;
	
	return RegisterDBService();
}


BOOL SFDBProcessorMySQL::RegisterDBService()
{
	m_Dispatch.RegisterMessage(DBMSG_LOGIN, std::tr1::bind(&SFDBProcessorMySQL::OnLogin, this, std::tr1::placeholders::_1));

	return TRUE;
}

BasePacket* SFDBProcessorMySQL::Alloc()
{
	return NULL;
}

BOOL SFDBProcessorMySQL::Release(BasePacket* pMessage)
{
	LogicEntrySingleton::instance()->ReleaseDBMessage((SFMessage*)pMessage);

	return TRUE;
}

BOOL SFDBProcessorMySQL::Call(BasePacket* pMessage)
{
	return m_Dispatch.HandleMessage(pMessage->GetPacketID(), pMessage);

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
BOOL SFDBProcessorMySQL::OnLogin( BasePacket* pPacket )
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
	SFMessage* pMsg = SFSendDBRequest::GetInitMessage(pMessage->GetCommand(), pMessage->GetOwnerSerial());
	*pMsg << Result;
	SFSendDBRequest::SendToLogic(pMsg);
	
	return TRUE;
}
