// ProtocolClient.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "ProtocolCallback.h"
#include "SFNetworkEntry.h"
#include "SFPacketProtocol.h"
#include "SFCGSFPacketProtocol.h"
#include "AvroClientProtocol.h"
#include "ProtocolClientProtocol.h"
#include "SFMsgPackProtocol.h"
#include "input.h"

#pragma comment(lib, "EngineLayer.lib")

void testAvro();
void testProtobuf();
void testCGSF();
void testMsgPack();
void testJson();

int _tmain(int argc, _TCHAR* argv[])
{
	testProtobuf();
	
	return 0;
}

void testAvro()
{
	ProtocolCallback* pCallback = new ProtocolCallback();

	SFNetworkEntry::GetInstance()->Initialize(pCallback, new SFPacketProtocol<AvroClientProtocol>(MAX_IO_SIZE, MAX_PACKET_SIZE));
	SFNetworkEntry::GetInstance()->Run();

	ProcessInput(&ProcessAvroInput);

	SFNetworkEntry::GetInstance()->ShutDown();

}

void testProtobuf()
{
	ProtocolCallback* pCallback = new ProtocolCallback();

	SFNetworkEntry::GetInstance()->Initialize(pCallback, new SFPacketProtocol<ProtocolClientProtocol>(MAX_IO_SIZE, MAX_PACKET_SIZE));
	SFNetworkEntry::GetInstance()->Run();

	ProcessInput(&ProcessProtobufInput);

	SFNetworkEntry::GetInstance()->ShutDown();

}

void testCGSF()
{
	ProtocolCallback* pCallback = new ProtocolCallback();

	SFNetworkEntry::GetInstance()->Initialize(pCallback, new SFPacketProtocol<SFCGSFPacketProtocol>(MAX_IO_SIZE, MAX_PACKET_SIZE, CGSF_PACKET_OPTION));
	SFNetworkEntry::GetInstance()->Run();

	ProcessInput(&ProcessCGSFInput);

	SFNetworkEntry::GetInstance()->ShutDown();

}

void testMsgPack()
{
	ProtocolCallback* pCallback = new ProtocolCallback();

	SFNetworkEntry::GetInstance()->Initialize(pCallback, new SFPacketProtocol<SFMsgPackProtocol>(MAX_IO_SIZE, MAX_PACKET_SIZE));
	SFNetworkEntry::GetInstance()->Run();

	ProcessInput(&ProcessMsgPackInput);

	SFNetworkEntry::GetInstance()->ShutDown();

}

void testJson()
{

}