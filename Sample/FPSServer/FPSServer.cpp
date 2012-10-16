#include "stdafx.h"
#include "SFEngine.h"
#include <conio.h>
#include "SFLogicEntry.h"
#include "P2PServer.h"
#include "GoogleLog.h"
#include "SFTraining.h"
#include "SFFreeForAll.h"
#include "SFShouter.h"
#include "SFUtil.h"
#include "SFIni.h"

SFEngine<GoogleLog, INetworkEngine>* g_pEngine = NULL;

#ifdef _DEBUG
#pragma comment(lib, "aced.lib")
#else
#pragma comment(lib, "ace.lib")
#endif

#pragma comment(lib, "CasualGame.lib")
#pragma comment(lib, "EngineLayer.lib")

#define GAMEMODE_NONE 0
#define GAMEMODE_TRAINING 1
#define GAMEMODE_DEATHMATCH 2
#define GAMEMODE_FREEFORALL 3
#define GAMEMODE_SUDDENDEATH 4

HINSTANCE g_pP2PHandle = 0;


int _tmain(int argc, _TCHAR* argv[])
{
	g_pEngine = new SFEngine<GoogleLog, INetworkEngine>;

	SFLogicEntry* pLogicEntry = new SFLogicEntry();

	/*int MaxPacketPool = 1000;

	PacketPoolSingleton::instance()->Init(MaxPacketPool);*/

	
	////////////////////////////////////////////////////////////////////
//Game Mode
////////////////////////////////////////////////////////////////////
	pLogicEntry->AddGameMode(GAMEMODE_TRAINING, new SFTraining(GAMEMODE_TRAINING));
	pLogicEntry->AddGameMode(GAMEMODE_FREEFORALL, new SFFreeForAll(GAMEMODE_FREEFORALL));

	pLogicEntry->Initialize();


	if(FALSE == g_pEngine->CreateSystem("CGSFEngine.dll", pLogicEntry, true))
		return 0;

////////////////////////////////////////////////////////////////////
//Timer
////////////////////////////////////////////////////////////////////
	_TimerInfo Timer;
	Timer.TimerID = TIMER_1_SEC;
	Timer.Period = 1000;
	Timer.StartDelay = 5000;

	if(g_pEngine->GetNetworkEngine()->CheckTimerImpl())
	{
		g_pEngine->GetNetworkEngine()->CreateTimerTask(TIMER_1_SEC, 5000, 1000);
	}

	g_pP2PHandle = ::LoadLibrary(L"P2PServer.dll");

	if(g_pP2PHandle == NULL)
		return 0;

	ACTIVATEP2P_FUNC *pfuncActivate;
	pfuncActivate = (ACTIVATEP2P_FUNC *)::GetProcAddress( g_pP2PHandle, "ActivateP2P" );
	int Result = pfuncActivate();

	if(Result != 0)
		return 0;

	SFUtil::SetCurDirToModuleDir();
	
	/*SFShouter Shouter;
	_ShouterInfo* pInfo = Shouter.GetShouterInfo();
	pInfo->ShouterTitle = L"Notification";
	pInfo->StartTime = 0;
	pInfo->RepeatCount = 5;
	pInfo->SentenceInterval = 3000;
	pInfo->MessageInterval = 10000;
	pInfo->SentenceList.push_back(L"안녕하세요");
	pInfo->SentenceList.push_back(L"공지기능 테스트 중입니다");
	pInfo->SentenceList.push_back(L"Korea Game Conference");

	Shouter.Write(L"Shouter.xml");*/

	SFIni ini;
	
	WCHAR szIP[20];
	USHORT Port;

	ini.SetPathName(_T("./Connection.ini"));
	ini.GetString(L"ServerInfo",L"IP",szIP, 20);
	Port = ini.GetInt(L"ServerInfo",L"PORT",0);
	
	std::string str = StringConversion::ToASCII(szIP);
	g_pEngine->Start((char*)str.c_str(), Port);

	_getch();

	DEACTIVATEP2P_FUNC *pfuncDeactivate;
	pfuncDeactivate = (DEACTIVATEP2P_FUNC *)::GetProcAddress( g_pP2PHandle, "DeactivateP2P" );
	Result = pfuncDeactivate();

	::FreeLibrary(g_pP2PHandle);

	g_pEngine->ShutDown();

	return 0;
}