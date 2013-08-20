#include "StdAfx.h"
#include "SFPlayerLobby.h"
#include "SFPlayer.h"
#include "SFPacketStore.pb.h"
#include "PacketID.h"
#include "DBMsg.h"
#include "SFRoomManager.h"
#include "SFSendPacket.h"
#include "SFLobby.h"
#include "GamePacketStructure.h"

SFPlayerLobby::SFPlayerLobby(SFPlayer* pOwner, ePlayerState State)
: SFPlayerState(pOwner, State)
{
	m_Dispatch.RegisterMessage(CGSF::EnterRoom, std::tr1::bind(&SFPlayerLobby::OnEnterRoom, this, std::tr1::placeholders::_1));
	m_Dispatch.RegisterMessage(CGSF::CreateRoom, std::tr1::bind(&SFPlayerLobby::OnCreateRoom, this, std::tr1::placeholders::_1));
	m_Dispatch.RegisterMessage(CGSF::ChatReq, std::tr1::bind(&SFPlayerLobby::OnChatReq, this, std::tr1::placeholders::_1));
	m_Dispatch.RegisterMessage(CGSF::PlayerIP, std::tr1::bind(&SFPlayerLobby::OnPlayerIP, this, std::tr1::placeholders::_1));
}

SFPlayerLobby::~SFPlayerLobby(void)
{
}

BOOL SFPlayerLobby::OnEnter()
{
	SFRoomManager* pRoomManager = SFLogicEntry::GetLogicEntry()->GetRoomManager();
	pRoomManager->SendRoomPage(this);

	SendEnterLobby(GetOwner());

	SFLobby* pLobby =SFLogicEntry::GetLogicEntry()->GetLobby();

	pLobby->OnEnter(GetOwner());


	return TRUE;
}

BOOL SFPlayerLobby::OnLeave()
{
	SFLobby* pLobby = SFLogicEntry::GetLogicEntry()->GetLobby();

	pLobby->OnLeave(GetOwner());

	return TRUE;
}

BOOL SFPlayerLobby::ProcessPacket( BasePacket* pPacket )
{
	return m_Dispatch.HandleMessage(pPacket->GetPacketID(), pPacket);
}

BOOL SFPlayerLobby::ProcessDBResult( SFMessage* pMessage )
{
	return TRUE;
}

BOOL SFPlayerLobby::OnEnterRoom( BasePacket* pPacket )
{
	SFPlayer* pPlayer = GetOwner();
	
	_PeerInfo Info = pPlayer->GetIPInfo();
	if(Info.ExternalIP == 0)
		return FALSE;

	SFRoomManager* pRoomManager = SFLogicEntry::GetLogicEntry()->GetRoomManager();

	SFProtobufPacket<SFPacketStore::EnterRoom>* pEnterRoom = (SFProtobufPacket<SFPacketStore::EnterRoom>*)pPacket;


	int RoomIndex = pEnterRoom->GetData().roomindex();

	SFRoom* pRoom = pRoomManager->GetRoom(RoomIndex);

	if(pRoom == NULL)
		return FALSE;

	if(TRUE == pRoom->ProcessUserRequest(pPlayer, pPacket))
		SendEnterRoom(pPlayer, pEnterRoom->GetData().gamemode(), pEnterRoom->GetData().roomindex());
		//일단주석SendToClient(GetOwner(), CGSF::EnterRoom, pEnterRoom->getStructuredData(), pEnterRoom->GetData().ByteSize());

	return TRUE;
}

BOOL SFPlayerLobby::OnCreateRoom( BasePacket* pPacket )
{
	SFPlayer* pPlayer = GetOwner();
	_PeerInfo Info = pPlayer->GetIPInfo();
	if(Info.ExternalIP == 0)
		return FALSE;

	SFRoomManager* pRoomManager = SFLogicEntry::GetLogicEntry()->GetRoomManager();

	SFProtobufPacket<SFPacketStore::CreateRoom>* pEnterCreateRoom = (SFProtobufPacket<SFPacketStore::CreateRoom>*)pPacket;

	BOOL bResult = pRoomManager->OnCreateRoom(this, pEnterCreateRoom->GetData().gamemode());

	if(FALSE == bResult)
		return FALSE;

	pPlayer->ChangeState(PLAYER_STATE_ROOM);

	SendCreateRoom(pPlayer, pEnterCreateRoom->GetData().gamemode());

	return TRUE;
}

BOOL SFPlayerLobby::OnChatReq( BasePacket* pPacket )
{
	SFLobby* pLobby = SFLogicEntry::GetLogicEntry()->GetLobby();

	SFPlayer* pPlayer = GetOwner();

	SFProtobufPacket<SFPacketStore::ChatReq>* pChatReq = (SFProtobufPacket<SFPacketStore::ChatReq>*)pPacket;

	pLobby->OnChat(GetOwner(), pChatReq->GetData().message());

	return TRUE;
}

BOOL SFPlayerLobby::OnPlayerIP( BasePacket* pPacket )
{
	SFPlayer* pPlayer = GetOwner();

	SFProtobufPacket<SFPacketStore::PLAYER_IP>* pPlayerIP = (SFProtobufPacket<SFPacketStore::PLAYER_IP>*)pPacket;

	PlayerIPMsg Msg;
	SF_GETPACKET_ARG(&Msg, pPlayerIP->GetData().playerip(), PlayerIPMsg);

	_PeerInfo Info;
	Info.ExternalIP = Msg.ExternalIP;
	Info.ExternalPort = Msg.ExternalPort;
	Info.LocalIP = Msg.LocalIP;
	Info.LocalPort = Msg.LocalPort;

	pPlayer->SetIPInfo(Info);

	return TRUE;
}