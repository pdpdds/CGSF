// ChatClient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "SFEngine.h"
#include "TCPNetworkCallback.h"
#include <iostream>
#include "PacketID.h"
#include "SFClient.h"
#include "GoogleLog.h"
#include "ChatPacketEntry.h"
#include "SFPacket.h"
#include "SFBreakPad.h"
#include "SFMinidump.h"
#include "SFCustomHandler.h"
#include "SFBugTrap.h"
#include "SFMGFramework.h"

SFSYSTEM_CLIENT* g_pNetworkEngine = NULL;

#ifdef _DEBUG
#pragma comment(lib, "aced.lib")
#else
#pragma comment(lib, "ace.lib")
#endif

#pragma comment(lib, "BaseLib.lib")
#pragma comment(lib, "libprotobuf.lib")
#pragma comment(lib, "NetworkEngine.lib")

BOOL g_bEndNetworkHandler = FALSE;

static void NetworkHandler(void* Args)
{
	g_pNetworkEngine->Run(NULL);
	
	return;
}

int _tmain(int argc, _TCHAR* argv[])
{
	SFExceptionHandler* pHandler = new SFBugTrap();
	if(FALSE == pHandler->Install())
		return FALSE;

	ACE::init();

	g_pNetworkEngine = new SFSYSTEM_CLIENT();
	g_pNetworkEngine->CreateSystem();

//////////////////////////////////////////////////////////////////////
//메인 쓰레드와 패킷 처리 쓰레드를 구별할 시 방법1
//////////////////////////////////////////////////////////////////////
	ChatPacketEntry* pLogicEntry = new ChatPacketEntry();
	if(FALSE == g_pNetworkEngine->Run(pLogicEntry))
	{
		//g_pNetworkEngine->Stop();
		//delete g_pNetworkEngine;
		return 0;
	}
//////////////////////////////////////////////////////////////////////


	//TCPNetworkCallback* pNetworkCallback = new TCPNetworkCallback();
	//g_pNetworkEngine->SetNetworkCallback(pNetworkCallback);

	//ACE_Thread_Manager::instance()->spawn_n(1, (ACE_THR_FUNC)NetworkHandler, NULL, THR_NEW_LWP, ACE_DEFAULT_THREAD_PRIORITY, 1);


	while(g_pNetworkEngine->GetProcessing() == TRUE)
	{
		std::string ChatMessage;
		std::cin >> ChatMessage;

		ChatPacket::Chat PktChat;
		PktChat.set_chatmessage(ChatMessage);

		int BufSize = PktChat.ByteSize();

		char Buffer[2048] = {0,};

		if(BufSize != 0)
		{
			::google::protobuf::io::ArrayOutputStream os(Buffer, BufSize);
			PktChat.SerializeToZeroCopyStream(&os);
		}

		if(g_pNetworkEngine->GetProcessing())
			pLogicEntry->Send(CGSF::ChatReq, Buffer, BufSize);
	}

	g_bEndNetworkHandler = TRUE;

	g_pNetworkEngine->Stop();

	ACE::fini();

	delete pHandler;

	return 0;
}