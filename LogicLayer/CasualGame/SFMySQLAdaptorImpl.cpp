#include "stdafx.h"
#include "SFMySQLAdaptorImpl.h"
#include "SFMySQL.h"
#include "MySQLPool.h"
#include "DBMsg.h"
#include "SFDBPacketSystem.h"

SFMySQLAdaptorImpl::SFMySQLAdaptorImpl(void)
{
	
}


SFMySQLAdaptorImpl::~SFMySQLAdaptorImpl(void)
{
	
}

BOOL SFMySQLAdaptorImpl::RegisterDBService()
{
	m_Dispatch.RegisterMessage(DBMSG_LOGIN, std::tr1::bind(&SFMySQLAdaptorImpl::OnLogin, this, std::tr1::placeholders::_1));

	return TRUE;
}

bool SFMySQLAdaptorImpl::RecallDBMsg(BasePacket* pMessage)
{
	return SFDBPacketSystem<SFMessage>::GetInstance()->RecallDBMsg(pMessage);
}

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
BOOL SFMySQLAdaptorImpl::OnLogin( BasePacket* pPacket )
{
	SFMySQL* pMySQL = GetObject();
	MySQLPool* pMySqlPool = pMySQL->GetMySqlPool();
	SFMessage* pMessage = (SFMessage*)pPacket;

	int Result = -1;
	char userName[100];
	char password[100];
	*pMessage >> userName;
	*pMessage >> password;
	char szQuery[100];
	sprintf_s(szQuery, "SELECT * FROM tblLogin WHERE UserName = '%s' AND Password = '%s'", userName, password);

	sql::Connection *con;
	sql::Statement* stmt;
	sql::ResultSet* res;

	/*Get a connection from the pool*/
	if((con= pMySqlPool->GetConnectionFromPool()) == 0)
	{
		LOG(ERROR) << "mysql connection get fail!!";
		return FALSE;
	}

	try
	{
		con->setSchema("test");
		stmt = con->createStatement();
		res = stmt->executeQuery(szQuery);

		if (1 == res->rowsCount())
			Result = 0;

		delete res;
		delete stmt;
	}
	catch (sql::SQLException e)
	{
		LOG(ERROR) << "mysql query execute fail!!";
		return FALSE;
	}
	
	/*Release the connection back into the pool*/
	pMySqlPool->ReleaseConnectionToPool(con);

////////////////////////////////////////////////////////////////////////////////
//결과를 로직 쓰레드로 보내야 할 경우
////////////////////////////////////////////////////////////////////////////////
	SFMessage* pMsg = SFDBPacketSystem<SFMessage>::GetInstance()->GetInitMessage(pMessage->GetCommand(), pMessage->GetSerial());
	*pMsg << Result;
	SFEngine::GetInstance()->SendToLogic(pMsg);
	
	return TRUE;
}