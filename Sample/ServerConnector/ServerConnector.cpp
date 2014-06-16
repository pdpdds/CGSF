// ServerConnector.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "ServerConnectorLogicEntry.h"
#include "SFJsonProtocol.h"
#include "SFJsonPacket.h"

#pragma comment(lib, "EngineLayer.lib")

int _tmain(int argc, _TCHAR* argv[])
{
	ServerConnectorLogicEntry* pLogicEntry = new ServerConnectorLogicEntry();
	SFEngine::GetInstance()->Intialize(pLogicEntry, new SFPacketProtocol<SFJsonProtocol>);

	int listenerId = -1;
	listenerId = SFEngine::GetInstance()->AddConnector("127.0.0.1", 10000);
	SFASSERT(listenerId != -1);

	listenerId = SFEngine::GetInstance()->AddConnector("127.0.0.1", 10001);
	SFASSERT(listenerId != -1);

	listenerId = SFEngine::GetInstance()->AddConnector("127.0.0.1", 10002);
	SFASSERT(listenerId != -1);

	SFEngine::GetInstance()->Start(0, 10004);

	SFJsonPacket packet(1000);
	packet.GetData().Add("ECHO", "aaaaaaaaaaa");
	packet.SetSerial(listenerId);
	SFEngine::GetInstance()->SendRequest(&packet);

	google::FlushLogFiles(google::GLOG_INFO);

	getchar();

	SFEngine::GetInstance()->ShutDown();

	return 0;
}