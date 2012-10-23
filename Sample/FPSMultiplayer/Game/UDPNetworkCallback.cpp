#include "UDPNetworkCallback.h"
#include "Main.h"
#include "PacketID.h"
#include "GamePacketStructure.h"

UDPNetworkCallback::UDPNetworkCallback(void)
{
}

UDPNetworkCallback::~UDPNetworkCallback(void)
{
}

bool UDPNetworkCallback::HandleUDPNetworkMessage( const unsigned char* pData, unsigned int Length )
{
	NetworkMessage* pMessage = (NetworkMessage*)pData;

	g_engine->GetCurrentState()->HandleNetworkMessage( pMessage );

	return true;
}

bool UDPNetworkCallback::ReportMyIP( unsigned int LocalIP, unsigned short LocalPort, unsigned int ExternalIP, unsigned short ExternalPort )
{
	PlayerIPMsg Msg;
	Msg.msgid = CGSF::PlayerIP;
	Msg.ExternalIP = ExternalIP;
	Msg.ExternalPort = ExternalPort;
	Msg.LocalIP = LocalIP;
	Msg.LocalPort = LocalPort;


	g_engine->GetCurrentState()->HandleNetworkMessage( &Msg );


	return true;
}