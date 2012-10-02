#include "StdAfx.h"
#include "SFDatabaseProxyImpl.h"

SFDatabaseProxyImpl::SFDatabaseProxyImpl(SFDatabaseProxy* pProxy)
: m_pDataBaseProxy(pProxy)
{
}

SFDatabaseProxyImpl::~SFDatabaseProxyImpl(void)
{
}


BOOL SFDatabaseProxyImpl::Initialize()
{
	return m_pDataBaseProxy->Initialize();
}

BOOL SFDatabaseProxyImpl::SendDBRequest(SFMessage* pMessage)
{
	return m_pDataBaseProxy->SendDBRequest(pMessage);
}