#include "StdAfx.h"
#include "SFServer.h"
#include "INetworkFramework.h"

SFServer::SFServer(void)
{
}

SFServer::~SFServer(void)
{
}

BOOL SFServer::Run( INetworkFramework* pFramework, ILogicEntry* pLogic )
{	
	if(!pFramework->Initialize(NetworkFramework_SERVER))
		return FALSE;

	if(pFramework->Start(pLogic) == FALSE)
		return FALSE;

	return TRUE;
}
