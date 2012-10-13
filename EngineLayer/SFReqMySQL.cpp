#include "StdAfx.h"
#include "SFReqMySQL.h"
#include "SFDBRequest.h"
#include "SFDatabaseMySQL.h"
#include "SFMessage.h"
#include "DBMsg.h"

extern ACE_TSS<SFDatabaseMySQL> Database;

SFReqMySQL::SFReqMySQL(void)
{
}

SFReqMySQL::~SFReqMySQL(void)
{
}

BOOL SFReqMySQL::Call( SFMessage* pMessage )
{
	MYSQL_RES *sql_result = NULL;
//	MYSQL_ROW sql_row;

	if(pMessage->GetCommand() == DBMSG_LOGIN)
	{
		int Result = -1;
	/*	SFPacketStore::Login PktLogin;
		protobuf::io::ArrayInputStream is(pMessage->GetData(), pMessage->GetDataSize());
		PktLogin.ParseFromZeroCopyStream(&is);

		std::string username = PktLogin.username();*/

		char szQuery[100];
		sprintf_s(szQuery, "SELECT * FROM tblLogin WHERE UserName = '%s'", "cgsf");

		if(TRUE == Database->Execute(szQuery))
		{
			sql_result = mysql_store_result(Database->GetDBConnection());

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

		SFMessage* pMsg = LogicEntrySingleton::instance()->GetDBMessage();
		pMessage->Initialize(DBMSG_LOGIN);
		pMessage->SetOwnerSerial(pMsg->GetOwnerSerial());
		pMessage->SetPacketType(pMsg->GetPacketType());
		*pMessage << Result;

		SendToLogic(pMsg);
	}

	return TRUE;
}