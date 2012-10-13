// ChatServer.cpp : Defines the entry point for the console application.

#include "stdafx.h"
#include "SFEngine.h"
#include <conio.h>
#include "ChatLogicEntry.h"
#include "SFServer.h"
#include "GoogleLog.h"
#include "SFClient.h"
#include "SFMacro.h"
#include "SFMGFramework.h"
#include "SFEngine.h"
SFSYSTEM_SERVER* g_pEngine = NULL;

#ifdef _DEBUG
#pragma comment(lib, "aced.lib")
#else
#pragma comment(lib, "ace.lib")
#endif

#pragma comment(lib, "libprotobuf.lib")
#pragma comment(lib, "EngineLayer.lib")

HINSTANCE g_pP2PHandle = 0;

int _tmain(int argc, _TCHAR* argv[])
{
	ACE::init();

	g_pEngine = new SFSYSTEM_SERVER();
	g_pEngine->CreateSystem();

	ChatLogicEntry* pLogicEntry = new ChatLogicEntry();

	_TimerInfo Timer;
	Timer.TimerID = TIMER_1_SEC;
	Timer.Period = 1000;
	Timer.StartDelay = 5000;

	pLogicEntry->AddTimer(Timer);

	g_pEngine->Run(pLogicEntry);

	_getch();

	g_pEngine->Stop();

	ACE::fini();

	return 0;
}

