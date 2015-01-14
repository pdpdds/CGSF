#include "StdAfx.h"
#include "SFMSSQLAdaptorImpl.h"
#include "QueryIdentifier.h"
#include "SFMessage.h"
#include "SFObjectPool.h"
#include "SFDBPacketSystem.h"

SFMSSQLAdaptorImpl::SFMSSQLAdaptorImpl(void)
{
}

SFMSSQLAdaptorImpl::~SFMSSQLAdaptorImpl(void)
{
}

BOOL SFMSSQLAdaptorImpl::AddStatement()
{
	if (!GetObject()->RegisterStatement(m_Stmt_SPLoadUser))
		return FALSE;

	return TRUE;
}

BOOL SFMSSQLAdaptorImpl::RegisterDBService() 
{
	if(FALSE == AddStatement())
		return FALSE;

	m_Dispatch.RegisterMessage(DBMSG_LOADUSER, std::tr1::bind(&SFMSSQLAdaptorImpl::OnLoadUser, this, std::tr1::placeholders::_1));
	return TRUE;
}

bool SFMSSQLAdaptorImpl::RecallDBMsg(BasePacket* pMessage)
{
	return SFDBPacketSystem<SFMessage>::GetInstance()->RecallDBMsg(pMessage);
}

BOOL SFMSSQLAdaptorImpl::OnLoadUser( BasePacket* pMessage )
{
	char szUserName[MAX_USER_NAME + 1] = { 0, };

	if(!GetObject()->IsDBConnected())
		return FALSE;

	SFMessage* pMsg = (SFMessage*)pMessage;

	int nUserID = pMessage->GetSerial();
	*pMsg >> (char*)szUserName;

	m_Stmt_SPLoadUser.Init();

	SFQuery query(m_Stmt_SPLoadUser);

	m_Stmt_SPLoadUser.nUserID = nUserID;
	memcpy(m_Stmt_SPLoadUser.szUserName, szUserName, sizeof(char)* MAX_USER_NAME);

	if(!query.Execute())
	{
		return FALSE;
	}

	while (query.Fetch())
	{

	}

	return TRUE;
}
