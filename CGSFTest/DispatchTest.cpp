#include "stdafx.h"
#include "DispatchTest.h"
#include "SFDispatch.h"
#include "SFPacketHandler.h"


#define PACKET_NUM_AUTH 0x1000

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
	//Packet Declaration
	SFProtobufPacket<PacketCore::Auth> userAuth = SFProtobufPacket<PacketCore::Auth>(PACKET_NUM_AUTH);	
	userAuth.GetData().set_encryptionkey(0x12345678);

	ProcessCDispatch(&userAuth);
	ProcessCPPDispatch(&userAuth);
	
	return true;
}

void DispatchTest::ProcessCDispatch(BasePacket* userAuth)
{
	SFDispatch<USHORT, std::tr1::function<void(BasePacket*)>, BasePacket*> DispatchingSystem;
	DispatchingSystem.RegisterMessage(PACKET_NUM_AUTH, sampleFunc);

	DispatchingSystem.HandleMessage(PACKET_NUM_AUTH, userAuth);
}
	
void DispatchTest::ProcessCPPDispatch(BasePacket* userAuth)
{
	SFPacketHandler* pPacketHandler = new SFPacketHandler();

//Dispatching Declaration and Function mapping
	SFDispatch<USHORT, std::tr1::function<void(BasePacket*)>, BasePacket*> DispatchingSystem;
	DispatchingSystem.RegisterMessage(PACKET_NUM_AUTH, std::tr1::bind(&SFPacketHandler::OnAuth, pPacketHandler, std::tr1::placeholders::_1));
			
	DispatchingSystem.HandleMessage(PACKET_NUM_AUTH, userAuth);

	delete pPacketHandler;
}