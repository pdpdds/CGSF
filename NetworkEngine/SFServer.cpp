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

	m_pFramework = pFramework;

	return TRUE;
}

BOOL SFServer::Send(int Serial, SFPacket* pPacket)
{
	return m_pFramework->Send(Serial, pPacket);
}

BOOL SFServer::Send( int Serial, USHORT PacketID, char* pBuffer, int BufferSize )
{
	return m_pFramework->Send(Serial, PacketID, pBuffer, BufferSize);
}
