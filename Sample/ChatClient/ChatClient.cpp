// ChatClient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "SFSinglton.h"
#include "SFBridgeThread.h"
#include "SFEngine.h"
#include "TCPNetworkCallback.h"
#include <iostream>
#include "PacketID.h"
//#include "GoogleLog.h"
#include "ChatPacketEntry.h"
#include "ChatPacketJsonEntry.h"
#include "SFBreakPad.h"
#include "SFMinidump.h"
#include "SFCustomHandler.h"
#include "SFBugTrap.h"
#include "SFIni.h"
#include "StringConversion.h"
#include "SFProtobufPacket.h"
#include "ChatClientProtocol.h"
#include "SFPacketProtocol.h"
#include "SFCasualGameDispatcher.h"
#include "SFJsonProtocol.h"
#include "SFJsonPacket.h"
#include "VMemPool.h"

#pragma comment(lib, "liblzf.lib")
#pragma comment(lib, "zlib.lib")

SFEngine* g_pEngine = NULL;

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

	g_pEngine = new SFEngine();
	ChatPacketJsonEntry* pLogicEntry = new ChatPacketJsonEntry();
	g_pEngine->CreateEngine("CGSFEngine.dll", FALSE);
	g_pEngine->CreateLogicThread(pLogicEntry);

	IPacketProtocol* pProtocol = new SFPacketProtocol<SFJsonProtocol>;
	g_pEngine->SetPacketProtocol(pProtocol);

	ILogicDispatcher* pDispatcher = new SFCasualGameDispatcher();
	g_pEngine->SetLogicDispathcer(pDispatcher);

	SFIni ini;
	
	WCHAR szIP[20];
	USHORT Port;

	ini.SetPathName(_T("./Connection.ini"));
	ini.GetString(L"ServerInfo",L"IP",szIP, 20);
	Port = ini.GetInt(L"ServerInfo",L"PORT",0);
	
	std::string str = StringConversion::ToASCII(szIP);
	g_pEngine->Start((char*)str.c_str(), Port);

	while(/*g_pEngine->GetProcessing() == TRUE*/ TRUE)
	{
		std::string ChatMessage;
		std::cin >> ChatMessage;

		/*SFProtobufPacket<ChatPacket::Chat> request(CGSF::ChatReq);
		request.SetOwnerSerial(0);
		request.GetData().set_chatmessage(ChatMessage);

		if(request.GetData().ByteSize() != 0)
		{
			pLogicEntry->SendRequest(&request);
		}*/


		SFJsonPacket JsonPacket;
		JsonObjectNode& ObjectNode = JsonPacket.GetData();
		ObjectNode.Add("PacketId", 1234);
		ObjectNode.Add("chat", ChatMessage);
		JsonPacket.SetOwnerSerial(pLogicEntry->GetSerial());

		pLogicEntry->SendRequest(&JsonPacket);
		
	}

	g_pEngine->ShutDown();

	delete g_pEngine;
	delete pLogicEntry; //책임질 대상을 결정할 것...

	ACE::fini();

	delete pHandler;

	return 0;
}