#include "stdafx.h"
#include "DispatchTest.h"
#include "SFDispatch.h"
#include "SFPacketHandler.h"

//샘플 패킷
#define PACKET_NUM_AUTH 1000
//C 형태의 글로벌 함수
void sampleFunc(BasePacket* packet)
{
	SFProtobufPacket<PacketCore::Auth>* pAuth = (SFProtobufPacket<PacketCore::Auth>*)packet;
	DWORD dwEncryptKey = pAuth->GetData().encryptionkey();

	printf("Encrypt Key : %x\n", dwEncryptKey);	
}

DispatchTest::DispatchTest(void)
{
}


DispatchTest::~DispatchTest(void)
{
}

//////////////////////////////////////////////////////////////////////
//Function Dispatch Test
//////////////////////////////////////////////////////////////////////
bool DispatchTest::Run()
{
	//가상의 패킷 선언
	SFProtobufPacket<PacketCore::Auth> userAuth = SFProtobufPacket<PacketCore::Auth>(PACKET_NUM_AUTH);	
	userAuth.GetData().set_encryptionkey(0x12345678);

	ProcessCDispatch(&userAuth);
	ProcessCPPDispatch(&userAuth);
	
	return true;
}

void DispatchTest::ProcessCDispatch(BasePacket* userAuth)
{
	//패킷번호와 글로벌 함수와의 매핑
	SFDispatch<USHORT, std::tr1::function<void(BasePacket*)>, BasePacket*> DispatchingSystem;
	DispatchingSystem.RegisterMessage(PACKET_NUM_AUTH, sampleFunc);

	DispatchingSystem.HandleMessage(PACKET_NUM_AUTH, userAuth);
}
	
void DispatchTest::ProcessCPPDispatch(BasePacket* userAuth)
{
	//패킷 핸들러 객체 선언
	SFPacketHandler* pPacketHandler = new SFPacketHandler();

	//패킷 번호와 객체의 메소드와 매핑
	SFDispatch<USHORT, std::tr1::function<void(BasePacket*)>, BasePacket*> DispatchingSystem;
	DispatchingSystem.RegisterMessage(PACKET_NUM_AUTH, std::tr1::bind(&SFPacketHandler::OnAuth, pPacketHandler, std::tr1::placeholders::_1));
			
	DispatchingSystem.HandleMessage(PACKET_NUM_AUTH, userAuth);

	delete pPacketHandler;
}