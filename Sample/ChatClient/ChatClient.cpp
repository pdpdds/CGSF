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

int _tmain(int argc, _TCHAR* argv[])
{
	SFExceptionHandler* pHandler = new SFBugTrap();
	if(FALSE == pHandler->Install())
		return FALSE;

	ACE::init();

	g_pNetworkEngine = new SFSYSTEM_CLIENT();
	g_pNetworkEngine->CreateSystem();

	ChatPacketEntry* pLogicEntry = new ChatPacketEntry();
	if(FALSE == g_pNetworkEngine->Run(pLogicEntry))
	{
		return 0;
	}

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

		pLogicEntry->Send(CGSF::ChatReq, Buffer, BufSize);
	}

	g_pNetworkEngine->Stop();

	delete g_pNetworkEngine;
	delete pLogicEntry; //책임질 대상을 결정할 것...

	ACE::fini();

	delete pHandler;

	return 0;
}