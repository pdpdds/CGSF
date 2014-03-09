#include "stdafx.h"
#include "UDPCallback.h"
#include "GamePacketStructure.h"
#include "SFConstant.h"
#include "SFStructure.h"
#include <SFPacketStore/SFPacketID.h>
#include <SFPacketStore/SFPacketStore.pb.h>
#include "BasePacket.h"
#include "SFProtobufPacket.h"
#include "CasualGameManager.h"
#include "NetworkSystem.h"
#include "CasualGameGUI.h"
#include "IGameMain.h"

extern IGameMain* g_pGameMain;
extern CasualGameManager* g_pCasualGameManager;

UDPCallback::UDPCallback(void)
{
}

UDPCallback::~UDPCallback(void)
{
}

bool UDPCallback::HandleUDPNetworkMessage( const unsigned char* pData, unsigned int Length )
{
	NetworkMessage* pMessage = (NetworkMessage*)pData;
	g_pGameMain->Notify( pMessage );

	return true;
}

bool UDPCallback::ReportMyIP( unsigned int LocalIP, unsigned short LocalPort, unsigned int ExternalIP, unsigned short ExternalPort )
{
	if(NULL == g_pCasualGameManager)
		return false;
	PlayerIPMsg Msg;
	Msg.msgid = CGSF::PlayerIP;
	Msg.ExternalIP = ExternalIP;
	Msg.ExternalPort = ExternalPort;
	Msg.LocalIP = LocalIP;
	Msg.LocalPort = LocalPort;

	SFProtobufPacket<SFPacketStore::PLAYER_IP> pktPlayerIP(CGSF::PlayerIP);
	pktPlayerIP.GetData().set_playerip(&Msg, sizeof(PlayerIPMsg));

	if(g_pCasualGameManager->GetNetwork())
		g_pCasualGameManager->GetNetwork()->TCPSend(&pktPlayerIP);
	
	

	return true;
}