// ProtocolServer.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "ProtocolLogicEntry.h"
#include "SFCGSFPacketProtocol.h"
#include "ProtocolCGSFHandler.h"
#include "ProtocolProtobufHandler.h"
#include "ProtocolServerProtocol.h"
#include "SFMsgPackProtocol.h"
#include "ProtocolMsgPackHandler.h"
#include "ProtocolAvroProtocol.h"
#include "ProtocolAvroHandler.h"

#pragma comment(lib, "EngineLayer.lib")

void testAvro();
void testProtobuf();
void testCGSF();
void testMsgPack();
void testJson();

int _tmain(int argc, _TCHAR* argv[])
{
	testProtobuf();
	
	SFEngine::GetInstance()->Start();

	google::FlushLogFiles(google::GLOG_INFO);

	getchar();

	SFEngine::GetInstance()->ShutDown();

	return 0;
}

void testAvro()
{
	ProtocolLogicEntry<ProtocolAvroHandler>* pLogicEntry = new ProtocolLogicEntry<ProtocolAvroHandler>();
	SFEngine::GetInstance()->Intialize(pLogicEntry, new SFPacketProtocol<ProtocolAvroProtocol>(MAX_IO_SIZE, MAX_PACKET_SIZE));
}

void testProtobuf()
{
	ProtocolLogicEntry<ProtocolProtobufHandler>* pLogicEntry = new ProtocolLogicEntry<ProtocolProtobufHandler>();
	SFEngine::GetInstance()->Intialize(pLogicEntry, new SFPacketProtocol<ProtocolServerProtocol>);

}
void testCGSF()
{
	SFBaseProtocol::SetPacketOption(CGSF_PACKET_OPTION);
	ProtocolLogicEntry<ProtocolCGSFHandler>* pLogicEntry = new ProtocolLogicEntry<ProtocolCGSFHandler>();
	SFEngine::GetInstance()->Intialize(pLogicEntry, new SFPacketProtocol<SFCGSFPacketProtocol>(MAX_IO_SIZE, MAX_PACKET_SIZE));
}
void testMsgPack()
{
	ProtocolLogicEntry<ProtocolMsgPackHandler>* pLogicEntry = new ProtocolLogicEntry<ProtocolMsgPackHandler>();
	SFEngine::GetInstance()->Intialize(pLogicEntry, new SFPacketProtocol<SFMsgPackProtocol>(MAX_IO_SIZE, MAX_PACKET_SIZE));
}

void testJson()
{

}