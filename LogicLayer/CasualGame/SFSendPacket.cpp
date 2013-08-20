#include "StdAfx.h"
#include "SFSendPacket.h"
#include "SFPacketStore.pb.h"
#include "PacketCore.pb.h"
#include "PacketID.h"
#include "SFPlayer.h"
#include "SFRoom.h"
#include "GamePacketStructure.h"
#include "SFP2PSys.h"
#include "SFProtobufPacket.h"

BOOL SendAuthPacket(int Serial)
{
	SFProtobufPacket<PacketCore::Auth> Auth = SFProtobufPacket<PacketCore::Auth>(CGSF::Auth);
	Auth.SetOwnerSerial(Serial);
	Auth.SetPacketID(CGSF::Auth);

	Auth.GetData().set_encryptionkey(ENCRYPTION_KEY);

	SFLogicEntry::GetLogicEntry()->SendRequest(&Auth);

	return TRUE;
}

BOOL SendLoginResult( SFPlayer* pPlayer,int Result )
{
	_UserInfo* pUserInfo = pPlayer->GetUserInfo();	
	pUserInfo->Serial = pPlayer->GetSerial();

	if(Result == 0)
	{
		SFProtobufPacket<SFPacketStore::LoginSuccess> LoginSuccess = SFProtobufPacket<SFPacketStore::LoginSuccess>(CGSF::LoginSuccess);
		LoginSuccess.SetOwnerSerial(pPlayer->GetSerial());
	
		LoginSuccess.GetData().set_result(Result);
		LoginSuccess.GetData().set_userinfo((const char*)pPlayer->GetUserInfo(), sizeof(_UserInfo));
		
		SFLogicEntry::GetLogicEntry()->SendRequest(&LoginSuccess);
	}
	else
	{
		SFProtobufPacket<SFPacketStore::LoginFail> LoginFail = SFProtobufPacket<SFPacketStore::LoginFail>(CGSF::LoginFail);
		LoginFail.SetOwnerSerial(pPlayer->GetSerial());
		LoginFail.GetData().set_result(Result);
		
		SFLogicEntry::GetLogicEntry()->SendRequest(&LoginFail);
	}

	return TRUE;
}

BOOL SendToClient(SFPlayer* pPlayer, BasePacket* pPacket)
{
	pPacket->SetOwnerSerial(pPlayer->GetSerial());
	SFLogicEntry::GetLogicEntry()->SendRequest(pPacket);
	return TRUE;
}

BOOL SendPlayStart( SFPlayer* pPlayer )
{
	SFProtobufPacket<SFPacketStore::PlayStart> PlayStart = SFProtobufPacket<SFPacketStore::PlayStart>(CGSF::PlayStart);
	PlayStart.SetOwnerSerial(pPlayer->GetSerial());
	SFLogicEntry::GetLogicEntry()->SendRequest(&PlayStart);

	return TRUE;
}

BOOL SendPlayEnd( SFPlayer* pPlayer )
{
	SFProtobufPacket<SFPacketStore::PlayEnd> PlayEnd = SFProtobufPacket<SFPacketStore::PlayEnd>(CGSF::PlayEnd);
	PlayEnd.SetOwnerSerial(pPlayer->GetSerial());
	SFLogicEntry::GetLogicEntry()->SendRequest(&PlayEnd);

	return TRUE;
}

BOOL SendEnterLobby( SFPlayer* pPlayer )
{
	SFProtobufPacket<SFPacketStore::EnterLobby> EnterLobby = SFProtobufPacket<SFPacketStore::EnterLobby>(CGSF::EnterLobby);
	EnterLobby.SetOwnerSerial(pPlayer->GetSerial());
	SFLogicEntry::GetLogicEntry()->SendRequest(&EnterLobby);

	return TRUE;
}

BOOL SendCreateRoom( SFPlayer* pPlayer, int GameMode )
{
	SFProtobufPacket<SFPacketStore::CreateRoom> CreateRoom = SFProtobufPacket<SFPacketStore::CreateRoom>(CGSF::CreateRoom);
	CreateRoom.SetOwnerSerial(pPlayer->GetSerial());
	CreateRoom.GetData().set_gamemode(GameMode);
	SFLogicEntry::GetLogicEntry()->SendRequest(&CreateRoom);

	return TRUE;
}

BOOL SendEnterRoom( SFPlayer* pPlayer, int GameMode, int RoomIndex )
{
	SFProtobufPacket<SFPacketStore::EnterRoom> EnterRoom = SFProtobufPacket<SFPacketStore::EnterRoom>(CGSF::EnterRoom);
	EnterRoom.SetOwnerSerial(pPlayer->GetSerial());
	EnterRoom.GetData().set_gamemode(GameMode);
	EnterRoom.GetData().set_roomindex(RoomIndex);
	SFLogicEntry::GetLogicEntry()->SendRequest(&EnterRoom);

	return TRUE;
}

BOOL SendLeaveRoom( SFPlayer* pPlayer )
{
	SFProtobufPacket<SFPacketStore::LeaveRoom> LeaveRoom = SFProtobufPacket<SFPacketStore::LeaveRoom>(CGSF::LeaveRoom);
	LeaveRoom.SetOwnerSerial(pPlayer->GetSerial());
	SFLogicEntry::GetLogicEntry()->SendRequest(&LeaveRoom);

	return TRUE;
}

BOOL SendEnterTeamMember( SFPlayer* pPlayer, int PlayerIndex )
{
	return TRUE;
}

BOOL SendLeaveTeamMember( SFPlayer* pPlayer, int PlayerIndex )
{
	return TRUE;
}

BOOL SendLoadingStart( SFPlayer* pPlayer )
{
	SFProtobufPacket<SFPacketStore::StartGame> StartGame = SFProtobufPacket<SFPacketStore::StartGame>(CGSF::StartGame);
	StartGame.SetOwnerSerial(pPlayer->GetSerial());
	SFLogicEntry::GetLogicEntry()->SendRequest(&StartGame);

	return TRUE;
}

BOOL SendCreatePlayer( SFPlayer* pPlayer, SFRoom* pRoom, BOOL ExceptMe)
{
	SFRoom::RoomMemberMap MemberMap = pRoom->GetRoomMemberMap();

	SFRoom::RoomMemberMap::iterator iter = MemberMap.begin();

	for(;iter != MemberMap.end(); iter++)
	{
		SFPlayer* pTarget = iter->second;

		if(ExceptMe == TRUE)
		{
			if(pTarget == pPlayer)
				continue;
		}

		SFProtobufPacket<SFPacketStore::MSG_CREATE_PLAYER> MsgCreatePlayer = SFProtobufPacket<SFPacketStore::MSG_CREATE_PLAYER>(CGSF::MSG_CREATE_PLAYER);

		MsgCreatePlayer.SetOwnerSerial(pPlayer->GetSerial());
		//SFPacketStore::MSG_CREATE_PLAYER PktMsgCreatePlayer;
		MsgCreatePlayer.GetData().set_serial(pTarget->GetSerial());
		MsgCreatePlayer.GetData().set_spawnindex(pTarget->GetSpawnIndex());

		SFLogicEntry::GetLogicEntry()->SendRequest(&MsgCreatePlayer);
	}
	
	return TRUE;
}

BOOL SendDestroyPlayer( SFPlayer* pPlayer, int PlayerIndex)
{
	SFProtobufPacket<SFPacketStore::MSG_DESTROY_PLAYER> MsgDestroyPlayer = SFProtobufPacket<SFPacketStore::MSG_DESTROY_PLAYER>(CGSF::MSG_DESTROY_PLAYER);

	MsgDestroyPlayer.GetData().set_serial(PlayerIndex);
	MsgDestroyPlayer.SetOwnerSerial(pPlayer->GetSerial());

	SFLogicEntry::GetLogicEntry()->SendRequest(&MsgDestroyPlayer);

	return TRUE;
}

BOOL SendSpawnPlayer( SFPlayer* pPlayer,SFPlayer* pTarget)
{
	SFProtobufPacket<SFPacketStore::MSG_SPAWN_PLAYER> PktMsgSpawnPlayer = SFProtobufPacket<SFPacketStore::MSG_SPAWN_PLAYER>(CGSF::MSG_SPAWN_PLAYER);

	_CharacterInfo* pInfo = pTarget->GetCharacterInfo();

	SpawnPlayerMsg msg;
	PktMsgSpawnPlayer.SetOwnerSerial(pPlayer->GetSerial());
	msg.PlayerID = pTarget->GetSerial();
	msg.translation = pInfo->translation;

	PktMsgSpawnPlayer.GetData().set_spawnplayer(&msg, sizeof(SpawnPlayerMsg));

	SFLogicEntry::GetLogicEntry()->SendRequest(&PktMsgSpawnPlayer);
	return TRUE;
}

BOOL SendPlayerHealth( SFPlayer* pPlayer,SFPlayer* pTarget)
{
 	SFProtobufPacket<SFPacketStore::MSG_PLAYER_HEALTH> PktPlayerHealth = SFProtobufPacket<SFPacketStore::MSG_PLAYER_HEALTH>(CGSF::MSG_PLAYER_HEALTH);
	PktPlayerHealth.SetOwnerSerial(pPlayer->GetSerial());

	_CharacterInfo* pInfo = pTarget->GetCharacterInfo();

	PlayerHealthMsg msg;
	
	msg.PlayerID = pTarget->GetSerial();
	msg.health = pInfo->health;

	PktPlayerHealth.GetData().set_playerhealth(&msg, sizeof(SpawnPlayerMsg));

	SFLogicEntry::GetLogicEntry()->SendRequest(&PktPlayerHealth);
	
	return TRUE;
}

BOOL SendPeerInfo( SFPlayer* pPlayer, SFRoom* pRoom)
{
	SFProtobufPacket<SFPacketStore::PeerList> PktPeerList = SFProtobufPacket<SFPacketStore::PeerList>(CGSF::PeerList);
	PktPeerList.SetOwnerSerial(pPlayer->GetSerial());

	SFP2PSys* pP2PSys = pRoom->GetP2PSys();

	SFP2PSys::PeerMap PeerMap = pP2PSys->GetPeerMap();

	SFP2PSys::PeerMap::iterator iter = PeerMap.begin();

	for(;iter != PeerMap.end(); iter++)
	{
		_PeerInfo* pInfo = &(iter->second);

		if(pPlayer->GetSerial() == iter->first)
		{
			continue;
		}

		SFPacketStore::PeerList::PeerInfo* pPeer = PktPeerList.GetData().add_peer();
		pPeer->set_serial(iter->first);
		pPeer->set_info(&(iter->second), sizeof(_PeerInfo));
	}

	SFLogicEntry::GetLogicEntry()->SendRequest(&PktPeerList);
	
	return TRUE;
}

BOOL SendDeletePeer(SFPlayer* pPlayer, int PlayerIndex )
{
	SFProtobufPacket<SFPacketStore::DELETE_PEER> DELETE_PEER = SFProtobufPacket<SFPacketStore::DELETE_PEER>(CGSF::DeletePeer);
	DELETE_PEER.SetOwnerSerial(pPlayer->GetSerial());
	DELETE_PEER.GetData().set_serial(PlayerIndex); 

	SFLogicEntry::GetLogicEntry()->SendRequest(&DELETE_PEER);

	return TRUE;
}