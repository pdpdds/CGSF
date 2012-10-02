#include "StdAfx.h"
#include "SFDatabaseProxyLocal.h"

SFDatabaseProxyLocal::SFDatabaseProxyLocal(void)
{
}

SFDatabaseProxyLocal::~SFDatabaseProxyLocal(void)
{
}

BOOL SFDatabaseProxyLocal::Initialize()
{
	m_DBManager.activate();

	return TRUE;
}

BOOL SFDatabaseProxyLocal::SendDBRequest(SFMessage* pMessage)
{
	if(NULL == pMessage)
		return FALSE;

	SFDBRequest* pReq = (SFDBRequest*)m_DBManager.GetDBReq();

	if(NULL == pReq)
		return FALSE;

	pReq->AttachDBMessage(pMessage);

	m_DBManager.perform(pReq);

	return TRUE;
}