// ChatClient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "SFSinglton.h"
#include "SFBridgeThread.h"
#include "SFEngine.h"
#include "TCPNetworkCallback.h"
#include <iostream>
#include "PacketID.h"
#include "GoogleLog.h"
#include "ChatPacketEntry.h"
#include "SFBreakPad.h"
#include "SFMinidump.h"
#include "SFCustomHandler.h"
#include "SFBugTrap.h"
#include "SFIni.h"
#include "StringConversion.h"


#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#pragma comment(lib, "liblzf.lib")
#pragma comment(lib, "zlib.lib")

SFSYSTEM* g_pEngine = NULL;

#ifdef _DEBUG
#pragma comment(lib, "aced.lib")
#else
#pragma comment(lib, "ace.lib")
#endif

#pragma comment(lib, "BaseLayer.lib")
#pragma comment(lib, "libprotobuf.lib")
#pragma comment(lib, "EngineLayer.lib")

int _tmain(int argc, _TCHAR* argv[])
{
	SFExceptionHandler* pHandler = new SFBugTrap();
	if(FALSE == pHandler->Install())
		return FALSE;

	ACE::init();

	g_pEngine = new SFSYSTEM();
	ChatPacketEntry* pLogicEntry = new ChatPacketEntry();
	g_pEngine->CreateSystem("MGEngine.dll", pLogicEntry);

	SFIni ini;
	
	WCHAR szIP[20];
	USHORT Port;

	ini.SetPathName(_T("./Connection.ini"));
	ini.GetString(L"ServerInfo",L"IP",szIP, 20);
	Port = ini.GetInt(L"ServerInfo",L"PORT",0);
	
	std::string str = StringConversion::ToASCII(szIP);
	g_pEngine->Start((char*)str.c_str(), Port);

	while(g_pEngine->GetProcessing() == TRUE)
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

	g_pEngine->ShutDown();

	delete g_pEngine;
	delete pLogicEntry; //책임질 대상을 결정할 것...

	ACE::fini();

	delete pHandler;

	return 0;
}