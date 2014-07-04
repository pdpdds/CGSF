#include "StdAfx.h"
#include "SFSendPacket.h"
#include "SFPlayer.h"
#include "SFRoom.h"
#include "GamePacketStructure.h"
#include "SFP2PSys.h"
#include "SFProtobufPacket.h"

BOOL SendAuthPacket(int serial)
{
	SFProtobufPacket<PacketCore::Auth> auth = SFProtobufPacket<PacketCore::Auth>(CGSF::Auth);
	auth.SetSerial(serial);
	auth.SetPacketID(CGSF::Auth);

	auth.GetData().set_encryptionkey(ENCRYPTION_KEY);

	SFLogicEntry::GetLogicEntry()->SendRequest(&auth);

	return TRUE;
}

BOOL SendLoginResult( SFPlayer* pPlayer,int result )
{
	_UserInfo* pUserInfo = pPlayer->GetUserInfo();	
	pUserInfo->Serial = pPlayer->GetSerial();

	if (result == 0)
	{
		SFProtobufPacket<SFPacketStore::LoginSuccess> loginSuccess = SFProtobufPacket<SFPacketStore::LoginSuccess>(CGSF::LoginSuccess);
		loginSuccess.SetSerial(pPlayer->GetSerial());
	
		loginSuccess.GetData().set_result(result);
		loginSuccess.GetData().set_userinfo((const char*)pPlayer->GetUserInfo(), sizeof(_UserInfo));
		
		SFLogicEntry::GetLogicEntry()->SendRequest(&loginSuccess);
	}
	else
	{
		SFProtobufPacket<SFPacketStore::LoginFail> loginFail = SFProtobufPacket<SFPacketStore::LoginFail>(CGSF::LoginFail);
		loginFail.SetSerial(pPlayer->GetSerial());
		loginFail.GetData().set_result(result);
		
		SFLogicEntry::GetLogicEntry()->SendRequest(&loginFail);
	}

	return TRUE;
}

BOOL SendToClient(SFPlayer* pPlayer, BasePacket* pPacket)
{
	pPacket->SetSerial(pPlayer->GetSerial());
	SFLogicEntry::GetLogicEntry()->SendRequest(pPacket);
	return TRUE;
}

BOOL SendPlayStart( SFPlayer* pPlayer )
{
	SFProtobufPacket<SFPacketStore::PlayStart> playStart(CGSF::PlayStart);
	playStart.SetSerial(pPlayer->GetSerial());
	SFLogicEntry::GetLogicEntry()->SendRequest(&playStart);

	return TRUE;
}

BOOL SendPlayEnd( SFPlayer* pPlayer )
{
	SFProtobufPacket<SFPacketStore::PlayEnd> playEnd = SFProtobufPacket<SFPacketStore::PlayEnd>(CGSF::PlayEnd);
	playEnd.SetSerial(pPlayer->GetSerial());
	SFLogicEntry::GetLogicEntry()->SendRequest(&playEnd);

	return TRUE;
}

BOOL SendEnterLobby( SFPlayer* pPlayer )
{
	SFProtobufPacket<SFPacketStore::EnterLobby> enterLobby = SFProtobufPacket<SFPacketStore::EnterLobby>(CGSF::EnterLobby);
	enterLobby.SetSerial(pPlayer->GetSerial());
	SFLogicEntry::GetLogicEntry()->SendRequest(&enterLobby);

	return TRUE;
}

BOOL SendCreateRoom( SFPlayer* pPlayer, int GameMode )
{
	SFProtobufPacket<SFPacketStore::CreateRoom> createRoom = SFProtobufPacket<SFPacketStore::CreateRoom>(CGSF::CreateRoom);
	createRoom.SetSerial(pPlayer->GetSerial());
	createRoom.GetData().set_gamemode(GameMode);
	SFLogicEntry::GetLogicEntry()->SendRequest(&createRoom);

	return TRUE;
}

BOOL SendEnterRoom( SFPlayer* pPlayer, int gameMode, int roomIndex )
{
	SFProtobufPacket<SFPacketStore::EnterRoom> enterRoom = SFProtobufPacket<SFPacketStore::EnterRoom>(CGSF::EnterRoom);
	enterRoom.SetSerial(pPlayer->GetSerial());
	enterRoom.GetData().set_gamemode(gameMode);
	enterRoom.GetData().set_roomindex(roomIndex);
	SFLogicEntry::GetLogicEntry()->SendRequest(&enterRoom);

	return TRUE;
}

BOOL SendLeaveRoom( SFPlayer* pPlayer )
{
	SFProtobufPacket<SFPacketStore::LeaveRoom> leaveRoom = SFProtobufPacket<SFPacketStore::LeaveRoom>(CGSF::LeaveRoom);
	leaveRoom.SetSerial(pPlayer->GetSerial());
	SFLogicEntry::GetLogicEntry()->SendRequest(&leaveRoom);

	return TRUE;
}

BOOL SendRoomMember(SFPlayer* pPlayer, SFRoom* pRoom)
{
	SFProtobufPacket<SFPacketStore::RoomMember> roomMember = SFProtobufPacket<SFPacketStore::RoomMember>(CGSF::RoomMember);
	roomMember.SetSerial(pPlayer->GetSerial());

	
		SFRoom::RoomMemberMap::iterator iter = pRoom->GetRoomMemberMap().begin();

		for(;iter != pRoom->GetRoomMemberMap().end(); iter++)
		{
			SFPlayer* pTarget = iter->second;

			if(pPlayer->GetSerial() == pTarget->GetSerial())
				continue;

			SFPacketStore::RoomMember::Member* pMember = roomMember.GetData().add_info();
			
			pMember->set_playerindex(pTarget->GetSerial());
			pMember->set_member(pTarget->GetPlayerName());
			pMember->set_type(SFPacketStore::TeamType(pTarget->GetMyTeam()));
		}
	

	SFLogicEntry::GetLogicEntry()->SendRequest(&roomMember);

	return TRUE;
}

BOOL SendEnterTeamMember( SFPlayer* pPlayer, int playerIndex, char* szPlayerName )
{
	SFProtobufPacket<SFPacketStore::EnterTeamMember> enterTeamMember = SFProtobufPacket<SFPacketStore::EnterTeamMember>(CGSF::EnterTeamMember);
	enterTeamMember.SetSerial(pPlayer->GetSerial());
	enterTeamMember.GetData().set_member(szPlayerName);
	enterTeamMember.GetData().set_playerindex(playerIndex);
	enterTeamMember.GetData().set_type(SFPacketStore::TeamType(0));

	SFLogicEntry::GetLogicEntry()->SendRequest(&enterTeamMember);

	return TRUE;
}

BOOL SendLeaveTeamMember( SFPlayer* pPlayer, int playerIndex, char* szPlayerName )
{
	SFProtobufPacket<SFPacketStore::LeaveTeamMember> leaveTeamMember = SFProtobufPacket<SFPacketStore::LeaveTeamMember>(CGSF::LeaveTeamMember);
	leaveTeamMember.SetSerial(pPlayer->GetSerial());
	
	leaveTeamMember.GetData().set_member(szPlayerName);
	leaveTeamMember.GetData().set_playerindex(playerIndex);
	leaveTeamMember.GetData().set_type(SFPacketStore::TeamType(0));

	SFLogicEntry::GetLogicEntry()->SendRequest(&leaveTeamMember);
	return TRUE;
}

BOOL SendLoadingStart( SFPlayer* pPlayer )
{
	SFProtobufPacket<SFPacketStore::LoadingStart> loadingStart = SFProtobufPacket<SFPacketStore::LoadingStart>(CGSF::LoadingStart);
	loadingStart.SetSerial(pPlayer->GetSerial());
	SFLogicEntry::GetLogicEntry()->SendRequest(&loadingStart);

	return TRUE;
}

BOOL SendCreatePlayer( SFPlayer* pPlayer, SFRoom* pRoom, BOOL exceptMe)
{
	SFRoom::RoomMemberMap MemberMap = pRoom->GetRoomMemberMap();

	SFRoom::RoomMemberMap::iterator iter = MemberMap.begin();

	for(;iter != MemberMap.end(); iter++)
	{
		SFPlayer* pTarget = iter->second;

		if (exceptMe == TRUE)
		{
			if(pTarget == pPlayer)
				continue;
		}

		SFProtobufPacket<SFPacketStore::MSG_CREATE_PLAYER> msgCreatePlayer = SFProtobufPacket<SFPacketStore::MSG_CREATE_PLAYER>(CGSF::MSG_CREATE_PLAYER);

		msgCreatePlayer.SetSerial(pPlayer->GetSerial());
		//SFPacketStore::MSG_CREATE_PLAYER PktMsgCreatePlayer;
		msgCreatePlayer.GetData().set_serial(pTarget->GetSerial());
		msgCreatePlayer.GetData().set_spawnindex(pTarget->GetSpawnIndex());

		SFLogicEntry::GetLogicEntry()->SendRequest(&msgCreatePlayer);
	}
	
	return TRUE;
}

BOOL SendDestroyPlayer( SFPlayer* pPlayer, int playerIndex)
{
	SFProtobufPacket<SFPacketStore::MSG_DESTROY_PLAYER> msgDestroyPlayer = SFProtobufPacket<SFPacketStore::MSG_DESTROY_PLAYER>(CGSF::MSG_DESTROY_PLAYER);

	msgDestroyPlayer.GetData().set_serial(playerIndex);
	msgDestroyPlayer.SetSerial(pPlayer->GetSerial());

	SFLogicEntry::GetLogicEntry()->SendRequest(&msgDestroyPlayer);

	return TRUE;
}

BOOL SendPeerInfo( SFPlayer* pPlayer, SFRoom* pRoom)
{
	SFProtobufPacket<SFPacketStore::PeerList> pktPeerList = SFProtobufPacket<SFPacketStore::PeerList>(CGSF::PeerList);
	pktPeerList.SetSerial(pPlayer->GetSerial());

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

		SFPacketStore::PeerList::PeerInfo* pPeer = pktPeerList.GetData().add_peer();
		pPeer->set_serial(iter->first);
		pPeer->set_info(&(iter->second), sizeof(_PeerInfo));
	}

	SFLogicEntry::GetLogicEntry()->SendRequest(&pktPeerList);
	
	return TRUE;
}

BOOL SendDeletePeer(SFPlayer* pPlayer, int playerIndex )
{
	SFProtobufPacket<SFPacketStore::DELETE_PEER> deletePeer = SFProtobufPacket<SFPacketStore::DELETE_PEER>(CGSF::DeletePeer);
	deletePeer.SetSerial(pPlayer->GetSerial());
	deletePeer.GetData().set_serial(playerIndex);

	SFLogicEntry::GetLogicEntry()->SendRequest(&deletePeer);

	return TRUE;
}