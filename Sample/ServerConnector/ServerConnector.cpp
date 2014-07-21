// ServerConnector.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "ServerConnectorLogicEntry.h"
#include "SFJsonProtocol.h"
#include "SFJsonPacket.h"
#include "SFServerConnectionManager.h"
#include "AuthServerCallback.h"
#include "DatabaseServerCallback.h"

#pragma comment(lib, "EngineLayer.lib")

#define GAME_DATABASE_SERVER_1 10000
#define AUTH_SERVER_1 9000

#define PACKET_PROTOCOL_JSON_1		1
#define PACKET_PROTOCOL_JSON_2		2
#define PACKET_PROTOCOL_PROTO_BUF	3

int _tmain(int argc, _TCHAR* argv[])
{
	ServerConnectorLogicEntry* pLogicEntry = new ServerConnectorLogicEntry();

//각각의 커넥터에 대한 콜백 객체를 추가한다.
	AuthServerCallback* pAuthServerCallback = new AuthServerCallback();
	DatabaseServerCallback* pDatabaseServerCallback = new DatabaseServerCallback();

	SFEngine::GetInstance()->Intialize(pLogicEntry);
	SFEngine::GetInstance()->AddPacketProtocol(PACKET_PROTOCOL_JSON_1, new SFPacketProtocol<SFJsonProtocol>);

	if (false == SFEngine::GetInstance()->LoadConnectorList(L"ServerConnection.xml"))
		return 0;

	pLogicEntry->AddConnectorCallback(AUTH_SERVER_1, pAuthServerCallback, PACKET_PROTOCOL_JSON_1);
	pLogicEntry->AddConnectorCallback(GAME_DATABASE_SERVER_1, pDatabaseServerCallback, PACKET_PROTOCOL_JSON_1);

	if (false == SFEngine::GetInstance()->SetupServerReconnectSys())
		return 0;


	//SFEngine::GetInstance()->Start(0, 10004);

	/*SFJsonPacket packet(1000);
	packet.GetData().Add("ECHO", "Server To Server Packet");
	packet.SetSerial(listenerId);
	SFEngine::GetInstance()->SendRequest(&packet);
	*/
	google::FlushLogFiles(google::GLOG_INFO);

	getchar();

	SFEngine::GetInstance()->ShutDown();

	return 0;
}