#include "stdafx.h"
#include "SFFastDBAdaptorImpl.h"
#include "QueryIdentifier.h"

SFFastDBAdaptorImpl::SFFastDBAdaptorImpl(void)
{
}


SFFastDBAdaptorImpl::~SFFastDBAdaptorImpl(void)
{
}

BOOL SFFastDBAdaptorImpl::RegisterDBService() 
{
	m_Dispatch.RegisterMessage(DBMSG_LOGIN, std::tr1::bind(&SFFastDBAdaptorImpl::OnLogin, this, std::tr1::placeholders::_1));
	return TRUE;
}

BOOL SFFastDBAdaptorImpl::OnLogin( BasePacket* pMessage )
{
	return TRUE;
}