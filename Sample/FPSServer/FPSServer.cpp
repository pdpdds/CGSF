#include "stdafx.h"
#include "SFEngine.h"
#include "GameModeConstant.h"
#include "SFLogicEntry.h"
#include "FPSProtocol.h"
#include "SFCasualGameDispatcher.h"

SFEngine* g_pEngine = NULL;

int _tmain(int argc, _TCHAR* argv[])
{
	g_pEngine = new SFEngine();
////////////////////////////////////////////////////////////////////
//Game Mode
////////////////////////////////////////////////////////////////////
	SFLogicEntry* pLogicEntry = new SFLogicEntry();
	pLogicEntry->AddGameMode(GAMEMODE_TRAINING, new SFTraining(GAMEMODE_TRAINING));
	pLogicEntry->AddGameMode(GAMEMODE_FREEFORALL, new SFFreeForAll(GAMEMODE_FREEFORALL));
	pLogicEntry->Initialize();

/////////////////////////////////////////////////////////////////////
	
	g_pEngine->Intialize(pLogicEntry, new SFPacketProtocol<FPSProtocol>, new SFCasualGameDispatcher());

	g_pEngine->Start();
	getchar();

	g_pEngine->ShutDown();

	return 0;
}





	
	
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

	/*SFIni ini;
	
	WCHAR szIP[20];
	USHORT Port;

	ini.SetPathName(_T("./Connection.ini"));
	ini.GetString(L"ServerInfo",L"IP",szIP, 20);
	Port = ini.GetInt(L"ServerInfo",L"PORT",0);
	
	std::string str = StringConversion::ToASCII(szIP);*/