#include "StdAfx.h"
#include "SFSendPacket.h"
#include "SFPacketStore.pb.h"
#include "PacketCore.pb.h"
#include "PacketID.h"
#include "SFPlayer.h"
#include "SFRoom.h"
#include "GamePacketStructure.h"
#include "SFP2PSys.h"

BOOL SendAuthPacket(int Serial)
{
	PacketCore::Auth PktAuth;
	PktAuth.set_encryptionkey(ENCRYPTION_KEY);

	int BufSize = PktAuth.ByteSize();

	char Buffer[2048] = {0,};

	if(BufSize != 0)
	{
		::google::protobuf::io::ArrayOutputStream os(Buffer, BufSize);
		PktAuth.SerializeToZeroCopyStream(&os);
	}

	SFLogicEntry::GetLogicEntry()->Send(Serial, CGSF::Auth, Buffer, BufSize);

	return TRUE;
}

BOOL SendToClient(SFPlayer* pPlayer, USHORT PacketID, ::google::protobuf::Message* pMessage, int BufSize)
{
	char Buffer[MAX_PACKET_DATA] = {0,};

	if(BufSize != 0)
	{	

		::google::protobuf::io::ArrayOutputStream os(Buffer, BufSize);
		if(false == pMessage->SerializeToZeroCopyStream(&os))
		{
			SFASSERT(0);
		}
	}

	SFLogicEntry::GetLogicEntry()->Send(pPlayer->GetSerial(), PacketID, Buffer, BufSize);

	return TRUE;
}

BOOL SendToClient(SFPlayer* pPlayer, SFPacket* pPacket)
{
	SFLogicEntry::GetLogicEntry()->Send(pPlayer, pPacket);
	return TRUE;
}

BOOL SendPlayStart( SFPlayer* pPlayer )
{
	return TRUE;
}

BOOL SendPlayEnd( SFPlayer* pPlayer )
{
	return TRUE;
}

BOOL SendLoginResult( SFPlayer* pPlayer,int Result )
{
	if(Result == 0)
	{
		SFPacketStore::LoginSuccess PktLoginSuccess;
		_UserInfo* pUserInfo = pPlayer->GetUserInfo();
		pUserInfo->Serial = pPlayer->GetSerial();

		PktLoginSuccess.set_result(Result);
		PktLoginSuccess.set_userinfo((const char*)pPlayer->GetUserInfo(), sizeof(_UserInfo));
		int BufSize = PktLoginSuccess.ByteSize();
		SendToClient(pPlayer, CGSF::LoginSuccess, &PktLoginSuccess, BufSize);
	}
	else
	{
		SFPacketStore::LoginFail PktLoginFail;
		PktLoginFail.set_result(Result);
		int BufSize = PktLoginFail.ByteSize();
		SendToClient(pPlayer, CGSF::LoginFail, &PktLoginFail, BufSize);
	}

	return TRUE;
}

BOOL SendEnterLobby( SFPlayer* pPlayer )
{
	SFPacketStore::EnterLobby PktEnterLobby;
	int BufSize = PktEnterLobby.ByteSize();
	SendToClient(pPlayer, CGSF::EnterLobby, &PktEnterLobby, BufSize);

	return TRUE;
}

BOOL SendLeaveRoom( SFPlayer* pPlayer )
{
	SFPacketStore::LeaveRoom PktLeaveRoom;
	int BufSize = PktLeaveRoom.ByteSize();
	SendToClient(pPlayer, CGSF::LeaveRoom, &PktLeaveRoom, BufSize);

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
	SFPacketStore::StartGame PktStartGame;
	int BufSize = PktStartGame.ByteSize();
	SendToClient(pPlayer, CGSF::StartGame, &PktStartGame, BufSize);
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

		SFPacketStore::MSG_CREATE_PLAYER PktMsgCreatePlayer;
		PktMsgCreatePlayer.set_serial(pTarget->GetSerial());
		PktMsgCreatePlayer.set_spawnindex(pTarget->GetSpawnIndex());

		int BufSize = PktMsgCreatePlayer.ByteSize();

		SendToClient(pPlayer, CGSF::MSG_CREATE_PLAYER, &PktMsgCreatePlayer, BufSize);
	}
	
	return TRUE;
}

BOOL SendDestroyPlayer( SFPlayer* pPlayer, int PlayerIndex)
{
	SFPacketStore::MSG_DESTROY_PLAYER PktMsgDestroyPlayer;
	PktMsgDestroyPlayer.set_serial(PlayerIndex);

	int BufSize = PktMsgDestroyPlayer.ByteSize();

	SendToClient(pPlayer, CGSF::MSG_DESTROY_PLAYER, &PktMsgDestroyPlayer, BufSize);
	
	return TRUE;
}

BOOL SendSpawnPlayer( SFPlayer* pPlayer,SFPlayer* pTarget)
{
	SFPacketStore::MSG_SPAWN_PLAYER PktMsgSpawnPlayer;

	_CharacterInfo* pInfo = pTarget->GetCharacterInfo();

	SpawnPlayerMsg msg;
	msg.PlayerID = pTarget->GetSerial();
	msg.translation = pInfo->translation;

	PktMsgSpawnPlayer.set_spawnplayer(&msg, sizeof(SpawnPlayerMsg));
	int BufSize = PktMsgSpawnPlayer.ByteSize();

	SendToClient(pPlayer, CGSF::MSG_SPAWN_PLAYER, &PktMsgSpawnPlayer, BufSize);

	return TRUE;
}

BOOL SendPlayerHealth( SFPlayer* pPlayer,SFPlayer* pTarget)
{
	SFPacketStore::MSG_PLAYER_HEALTH PktPlayerHealth;

	_CharacterInfo* pInfo = pTarget->GetCharacterInfo();

	PlayerHealthMsg msg;
	msg.PlayerID = pTarget->GetSerial();
	msg.health = pInfo->health;

	PktPlayerHealth.set_playerhealth(&msg, sizeof(SpawnPlayerMsg));
	int BufSize = PktPlayerHealth.ByteSize();

	SendToClient(pPlayer, CGSF::MSG_PLAYER_HEALTH, &PktPlayerHealth, BufSize);

	return TRUE;
}

BOOL SendPeerInfo( SFPlayer* pPlayer, SFRoom* pRoom)
{
	SFPacketStore::PeerList PktPeerList;

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

		SFPacketStore::PeerList::PeerInfo* pPeer = PktPeerList.add_peer();
		pPeer->set_serial(iter->first);
		pPeer->set_info(&(iter->second), sizeof(_PeerInfo));
	}

	int BufSize = PktPeerList.ByteSize();
	SendToClient(pPlayer, CGSF::PeerList, &PktPeerList, BufSize);

	return TRUE;
}

BOOL SendDeletePeer(SFPlayer* pPlayer, int PlayerIndex )
{
	SFPacketStore::DELETE_PEER PktDeletePeer;
	PktDeletePeer.set_serial(PlayerIndex);

	int BufSize = PktDeletePeer.ByteSize();

	SendToClient(pPlayer, CGSF::DeletePeer, &PktDeletePeer, BufSize);

	return TRUE;
}