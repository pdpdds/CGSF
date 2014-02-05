#include "stdafx.h"
#include "SFFastDBAdaptorImpl.h"


SFFastDBAdaptorImpl::SFFastDBAdaptorImpl(void)
{
}


SFFastDBAdaptorImpl::~SFFastDBAdaptorImpl(void)
{
}

BOOL SFFastDBAdaptorImpl::RegisterDBService() 
{
	return TRUE;
}

BOOL SFFastDBAdaptorImpl::OnLogin( BasePacket* pMessage )
{
	return TRUE;
}