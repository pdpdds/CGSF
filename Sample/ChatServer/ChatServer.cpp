// ChatServer.cpp : Defines the entry point for the console application.

#include "stdafx.h"
#include "SFEngine.h"
#include <conio.h>
#include "ChatLogicEntry.h"
#include "GoogleLog.h"
#include "SFMacro.h"
#include "SFSinglton.h"
#include "SFEngine.h"
#include "SFPacketProtocol.h"
#include "SFCasualGameDispatcher.h"
#include "ChatProtocol.h"

SFEngine* g_pEngine = NULL;

#ifdef _DEBUG
#pragma comment(lib, "aced.lib")
#else
#pragma comment(lib, "ace.lib")
#endif

#pragma comment(lib, "libprotobuf.lib")
#pragma comment(lib, "EngineLayer.lib")
#pragma comment(lib, "liblzf.lib")
#pragma comment(lib, "zlib.lib")

HINSTANCE g_pP2PHandle = 0;

int _tmain(int argc, _TCHAR* argv[])
{
	ACE::init();

	g_pEngine = new SFEngine();
	
	IPacketProtocol* pProtocol = new SFPacketProtocol<ChatProtocol>;
	g_pEngine->SetPacketProtocol(pProtocol);

	ChatLogicEntry* pLogicEntry = new ChatLogicEntry();
	g_pEngine->CreateEngine("CGSFEngine.dll", TRUE);
	g_pEngine->CreateLogicThread(pLogicEntry);

	ILogicDispatcher* pDispatcher = new SFCasualGameDispatcher();
	g_pEngine->SetLogicDispathcer(pDispatcher);

	/*_TimerInfo Timer;
	Timer.TimerID = TIMER_1_SEC;
	Timer.Period = 1000;
	Timer.StartDelay = 5000;

	pLogicEntry->AddTimer(Timer);*/

	g_pEngine->Start("127.0.0.1", 25251);

	_getch();

	g_pEngine->ShutDown();

	ACE::fini();

	return 0;
}

