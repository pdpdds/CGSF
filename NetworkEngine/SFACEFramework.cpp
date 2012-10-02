#include "StdAfx.h"
#include "SFACEFramework.h"
#include "SFProactorServer.h"
#include "SFProactorClient.h"
#include "SFTSSyncQueue.h"
#include "SFIOCPQueue.h"
#include "Mmsystem.h"

#pragma comment(lib, "zlib.lib")
#pragma comment(lib, "liblzf.lib")
#pragma comment(lib, "libprotobuf.lib")
//#pragma comment(lib, "tomcrypt.lib")
#ifdef _DEBUG
#pragma comment(lib, "aced.lib")
#else
#pragma comment(lib, "ace.lib")
#endif

#pragma comment(lib, "Winmm.lib")

SFACEFramework::SFACEFramework()
{
}

SFACEFramework::~SFACEFramework(void)
{
}

BOOL SFACEFramework::Initialize(eNetworkFrameworkType Type)
{
	ACE::init();

	if(NetworkFramework_MAX <= Type)
		return FALSE;

	SetNetworkFrameworkType(Type);

	if(Type == NetworkFramework_SERVER)
	{
		m_pNetworkAgent = new SFProactorServer();
	}
	else
	{
		m_pNetworkAgent = new SFProactorClient();
	}

	int MaxPacketPool = 1000;

	PacketPoolSingleton::instance()->Init(MaxPacketPool);

	return TRUE;
}

BOOL SFACEFramework::Start(ILogicEntry* pLogic)
{
	return m_pNetworkAgent->Start(pLogic);
}

BOOL SFACEFramework::End()
{
	return m_pNetworkAgent->End();
}

BOOL SFACEFramework::Finally()
{
	ACE::fini();

	return TRUE;
}