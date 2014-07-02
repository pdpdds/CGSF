#include "StdAfx.h"
#include "SFObserver.h"

#pragma warning (disable : 4100) 


SFObserver::SFObserver(void)
{
}

SFObserver::~SFObserver(void)
{
}

BOOL SFObserver::OnMessage( SFObject* pSender, SFMessage* pMessage )
{
	return TRUE;
}