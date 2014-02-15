#include "StdAfx.h"
#include "SFMSSQLAdaptorImpl.h"
#include "QueryIdentifier.h"

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

	m_Dispatch.RegisterMessage(DBMSG_BOOKINFO, std::tr1::bind(&SFMSSQLAdaptorImpl::OnLoadUser, this, std::tr1::placeholders::_1));	
	return TRUE;
}

BOOL SFMSSQLAdaptorImpl::OnLoadUser( BasePacket* pMessage )
{
	if(!GetObject()->IsDBConnected())
		return FALSE;

	SFMessage* pMsg = (SFMessage*)pMessage;

	int nUserID = 1234;
	TCHAR* szUserName = L"CGSF";

	m_Stmt_SPLoadUser.Init();

	SFQuery query(m_Stmt_SPLoadUser);

	m_Stmt_SPLoadUser.nUserID = nUserID;
	memcpy(m_Stmt_SPLoadUser.szUserName, szUserName, sizeof(TCHAR) * 10);

	if(!query.Execute())
	{
		return FALSE;
	}

	while (query.Fetch())
	{
	}

	return TRUE;
}
