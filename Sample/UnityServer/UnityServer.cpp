// UnityServer.cpp : Defines the entry point for the console application.
#include "stdafx.h"
#include "UnityServerEntry.h"
#include "SFJsonProtocol.h"
#include "SFCasualGameDispatcher.h"

#pragma comment(lib, "EngineLayer.lib")

PersonalInfoManager* PersonalInfoManager::ins = nullptr;

int _tmain(int argc, _TCHAR* argv[])
{
	UnityServerEntry* pLogicEntry = new UnityServerEntry();

	SFEngine::GetInstance()->Intialize(pLogicEntry);
	SFEngine::GetInstance()->AddPacketProtocol(0, new SFPacketProtocol<SFJsonProtocol>);

	SFEngine::GetInstance()->Start(0);

	getchar();

	SFEngine::GetInstance()->ShutDown();

	return 0;
}