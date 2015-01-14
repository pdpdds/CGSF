#include "stdafx.h"
#include "SFZdbAdaptorImpl.h"
#include "SFZdb.h"
#include "DBMsg.h"
#include "SFObjectPool.h"
#include "SFDBPacketSystem.h"
#include "SFMessage.h"

SFZdbAdaptorImpl::SFZdbAdaptorImpl()
{
}


SFZdbAdaptorImpl::~SFZdbAdaptorImpl()
{
}


BOOL SFZdbAdaptorImpl::RegisterDBService()
{
	m_Dispatch.RegisterMessage(DBMSG_LOGIN, std::tr1::bind(&SFZdbAdaptorImpl::OnLogin, this, std::tr1::placeholders::_1));

	return TRUE;
}

bool SFZdbAdaptorImpl::RecallDBMsg(BasePacket* pMessage)
{
	return SFDBPacketSystem<SFMessage>::GetInstance()->RecallDBMsg(pMessage);
}

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
BOOL SFZdbAdaptorImpl::OnLogin(BasePacket* pPacket)
{
	SFZdb* pZdb = GetObject();
	SFMessage* pMessage = (SFMessage*)pPacket;

	int result = -1;
	char userName[100];
	char password[100];
	*pMessage >> userName;
	*pMessage >> password;
	char szQuery[100];
	sprintf_s(szQuery, "SELECT * FROM tblLogin WHERE UserName = '%s' AND Password = '%s'", userName, password);

	Connection_T con = ConnectionPool_getConnection(pZdb->pool);

	TRY
	{
		ResultSet_T result = Connection_executeQuery(con, szQuery);

		if (ResultSet_next(result))
		{
			const char* szchar = ResultSet_getString(result, 1);
			const char *name = ResultSet_getString(result, 2);
			printf("%s : %s\n", szchar, name);
			//int blobSize;
			//const void *image = ResultSet_getBlob(result, 3, &blobSize);
						
			result = 0;
		}
	}
	CATCH(SQLException)
	{
		result = -1;
		printf("SQLException -- %s\n", Exception_frame.message);
	}
	FINALLY
	{
		Connection_close(con);
	}
	END_TRY;
	
	////////////////////////////////////////////////////////////////////////////////
	//결과를 로직 쓰레드로 보내야 할 경우
	////////////////////////////////////////////////////////////////////////////////
	//SFMessage* pMsg = SFDBPacketSystem<SFMessage>::GetInstance()->GetInitMessage(pMessage->GetCommand(), pMessage->GetSerial());
	//*pMsg << result;
	//SFDBPacketSystem<SFMessage>::GetInstance()->SendToLogic(pMsg);

	return TRUE;
}
