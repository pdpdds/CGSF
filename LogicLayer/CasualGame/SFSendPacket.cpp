#include "StdAfx.h"
#include "SFSendPacket.h"
#include "SFPlayer.h"
#include "SFRoom.h"
#include "GamePacketStructure.h"
#include "SFP2PSys.h"
#include "SFProtobufPacket.h"

BOOL SendAuthPacket(int Serial)
{
	SFProtobufPacket<PacketCore::Auth> Auth = SFProtobufPacket<PacketCore::Auth>(CGSF::Auth);
	Auth.SetSerial(Serial);
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
		LoginSuccess.SetSerial(pPlayer->GetSerial());
	
		LoginSuccess.GetData().set_result(Result);
		LoginSuccess.GetData().set_userinfo((const char*)pPlayer->GetUserInfo(), sizeof(_UserInfo));
		
		SFLogicEntry::GetLogicEntry()->SendRequest(&LoginSuccess);
	}
	else
	{
		SFProtobufPacket<SFPacketStore::LoginFail> LoginFail = SFProtobufPacket<SFPacketStore::LoginFail>(CGSF::LoginFail);
		LoginFail.SetSerial(pPlayer->GetSerial());
		LoginFail.GetData().set_result(Result);
		
		SFLogicEntry::GetLogicEntry()->SendRequest(&LoginFail);
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
	SFProtobufPacket<SFPacketStore::PlayEnd> PlayEnd = SFProtobufPacket<SFPacketStore::PlayEnd>(CGSF::PlayEnd);
	PlayEnd.SetSerial(pPlayer->GetSerial());
	SFLogicEntry::GetLogicEntry()->SendRequest(&PlayEnd);

	return TRUE;
}

BOOL SendEnterLobby( SFPlayer* pPlayer )
{
	SFProtobufPacket<SFPacketStore::EnterLobby> EnterLobby = SFProtobufPacket<SFPacketStore::EnterLobby>(CGSF::EnterLobby);
	EnterLobby.SetSerial(pPlayer->GetSerial());
	SFLogicEntry::GetLogicEntry()->SendRequest(&EnterLobby);

	return TRUE;
}

BOOL SendCreateRoom( SFPlayer* pPlayer, int GameMode )
{
	SFProtobufPacket<SFPacketStore::CreateRoom> CreateRoom = SFProtobufPacket<SFPacketStore::CreateRoom>(CGSF::CreateRoom);
	CreateRoom.SetSerial(pPlayer->GetSerial());
	CreateRoom.GetData().set_gamemode(GameMode);
	SFLogicEntry::GetLogicEntry()->SendRequest(&CreateRoom);

	return TRUE;
}

BOOL SendEnterRoom( SFPlayer* pPlayer, int GameMode, int RoomIndex )
{
	SFProtobufPacket<SFPacketStore::EnterRoom> EnterRoom = SFProtobufPacket<SFPacketStore::EnterRoom>(CGSF::EnterRoom);
	EnterRoom.SetSerial(pPlayer->GetSerial());
	EnterRoom.GetData().set_gamemode(GameMode);
	EnterRoom.GetData().set_roomindex(RoomIndex);
	SFLogicEntry::GetLogicEntry()->SendRequest(&EnterRoom);

	return TRUE;
}

BOOL SendLeaveRoom( SFPlayer* pPlayer )
{
	SFProtobufPacket<SFPacketStore::LeaveRoom> LeaveRoom = SFProtobufPacket<SFPacketStore::LeaveRoom>(CGSF::LeaveRoom);
	LeaveRoom.SetSerial(pPlayer->GetSerial());
	SFLogicEntry::GetLogicEntry()->SendRequest(&LeaveRoom);

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

BOOL SendEnterTeamMember( SFPlayer* pPlayer, int PlayerIndex, char* szPlayerName )
{
	SFProtobufPacket<SFPacketStore::EnterTeamMember> enterTeamMember = SFProtobufPacket<SFPacketStore::EnterTeamMember>(CGSF::EnterTeamMember);
	enterTeamMember.SetSerial(pPlayer->GetSerial());
	enterTeamMember.GetData().set_member(szPlayerName);
	enterTeamMember.GetData().set_playerindex(PlayerIndex);
	enterTeamMember.GetData().set_type(SFPacketStore::TeamType(0));

	SFLogicEntry::GetLogicEntry()->SendRequest(&enterTeamMember);

	return TRUE;
}

BOOL SendLeaveTeamMember( SFPlayer* pPlayer, int PlayerIndex, char* szPlayerName )
{
	SFProtobufPacket<SFPacketStore::LeaveTeamMember> leaveTeamMember = SFProtobufPacket<SFPacketStore::LeaveTeamMember>(CGSF::LeaveTeamMember);
	leaveTeamMember.SetSerial(pPlayer->GetSerial());
	
	leaveTeamMember.GetData().set_member(szPlayerName);
	leaveTeamMember.GetData().set_playerindex(PlayerIndex);
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

		MsgCreatePlayer.SetSerial(pPlayer->GetSerial());
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
	MsgDestroyPlayer.SetSerial(pPlayer->GetSerial());

	SFLogicEntry::GetLogicEntry()->SendRequest(&MsgDestroyPlayer);

	return TRUE;
}

BOOL SendPeerInfo( SFPlayer* pPlayer, SFRoom* pRoom)
{
	SFProtobufPacket<SFPacketStore::PeerList> PktPeerList = SFProtobufPacket<SFPacketStore::PeerList>(CGSF::PeerList);
	PktPeerList.SetSerial(pPlayer->GetSerial());

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
	DELETE_PEER.SetSerial(pPlayer->GetSerial());
	DELETE_PEER.GetData().set_serial(PlayerIndex); 

	SFLogicEntry::GetLogicEntry()->SendRequest(&DELETE_PEER);

	return TRUE;
}