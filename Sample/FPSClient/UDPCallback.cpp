#include "stdafx.h"
#include "UDPCallback.h"
#include "PacketID.h"
#include "GamePacketStructure.h"
#include "SFConstant.h"
#include "SFStructure.h"

UDPCallback::UDPCallback(void)
{
}

UDPCallback::~UDPCallback(void)
{
}

bool UDPCallback::HandleUDPNetworkMessage( const unsigned char* pData, unsigned int Length )
{
	//NetworkMessage* pMessage = (NetworkMessage*)pData;
	//g_engine->GetCurrentState()->HandleNetworkMessage( pMessage );

	return true;
}

bool UDPCallback::ReportMyIP( unsigned int LocalIP, unsigned short LocalPort, unsigned int ExternalIP, unsigned short ExternalPort )
{
	PlayerIPMsg Msg;
	Msg.msgid = CGSF::PlayerIP;
	Msg.ExternalIP = ExternalIP;
	Msg.ExternalPort = ExternalPort;
	Msg.LocalIP = LocalIP;
	Msg.LocalPort = LocalPort;

	//g_engine->GetCurrentState()->HandleNetworkMessage( &Msg );

	return true;
}