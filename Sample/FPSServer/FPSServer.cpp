#include "stdafx.h"
#include "SFEngine.h"
#include "GameModeConstant.h"
#include "SFLogicEntry.h"
#include "FPSProtocol.h"

int _tmain(int argc, _TCHAR* argv[])
{
	/*void ServiceTest(int argc, TCHAR* argv[])
{
#ifdef _DEBUG
	//SFFacade Facade;
	//Facade.Initialize();
#else
	SFServiceController Controller;
	Controller.ServiceEntry(L"CGSF");
#endif

	if(argc == 2)
	{
		SFServiceController Controller;

		if(_tcscmp(argv[1], L"-I") == 0)
		{
			Controller.InstallService(L"CGSF", L"CGSF", L"D:\\NPSvr\\CGSFTest.exe");
		}
		else if(_tcscmp(argv[1], L"-U") == 0)
		{
			Controller.DeleteService(L"CGSFTest");
		}
	}
}*/

////////////////////////////////////////////////////////////////////
//Game Mode
////////////////////////////////////////////////////////////////////
	SFLogicEntry* pLogicEntry = new SFLogicEntry();
	pLogicEntry->AddGameMode(GAMEMODE_TRAINING, new SFTraining(GAMEMODE_TRAINING));
	pLogicEntry->AddGameMode(GAMEMODE_FREEFORALL, new SFFreeForAll(GAMEMODE_FREEFORALL));
	
/////////////////////////////////////////////////////////////////////
	if (FALSE == SFEngine::GetInstance()->Intialize(pLogicEntry, new SFPacketProtocol<FPSProtocol>) ||
		FALSE == SFEngine::GetInstance()->Start())
	{
		google::FlushLogFiles(google::GLOG_INFO);
		google::FlushLogFiles(google::GLOG_ERROR);
		
		return 0;
	}

	google::FlushLogFiles(google::GLOG_INFO);

	getchar();

	SFEngine::GetInstance()->ShutDown();

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