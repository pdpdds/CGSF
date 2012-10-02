#include "stdafx.h"
#include "SFMGFramework.h"
#include "SFMGServer.h"
#include "SFMGClient.h"

#pragma comment(lib, "zlib.lib")
#pragma comment(lib, "liblzf.lib")
#pragma comment(lib, "libprotobuf.lib")

#pragma comment(lib, "libprotobuf.lib")

//#pragma comment(lib, "tomcrypt.lib")


#pragma comment(lib, "Winmm.lib")

SFMGFramework::SFMGFramework()
{
}

SFMGFramework::~SFMGFramework(void)
{
}

BOOL SFMGFramework::Initialize(eNetworkFrameworkType Type)
{
	
	if(NetworkFramework_MAX <= Type)
		return FALSE;

	SetNetworkFrameworkType(Type);

	if(Type == NetworkFramework_SERVER)
	{
		m_pNetworkAgent = new SFMGServer();
	}
	else
	{
		m_pNetworkAgent = new SFMGClient();
	}

	int MaxPacketPool = 1000;

	PacketPoolSingleton::instance()->Init(MaxPacketPool);

	return TRUE;
}

BOOL SFMGFramework::Start(ILogicEntry* pLogic)
{
	return m_pNetworkAgent->Start(pLogic);
}

BOOL SFMGFramework::End()
{
	return m_pNetworkAgent->End();
}

BOOL SFMGFramework::Finally()
{
	return TRUE;
}

BOOL SFMGFramework::Send(int Serial, USHORT PacketID, char* pMessage, int BufSize )
{
	ISFAgent* pAgent = GetNetworkAgent();
	return pAgent->Send(Serial, PacketID, pMessage, BufSize );
}

BOOL SFMGFramework::Send(int Serial, SFPacket* pPacket)
{
	ISFAgent* pAgent = GetNetworkAgent();
	return pAgent->Send(Serial, pPacket);
}