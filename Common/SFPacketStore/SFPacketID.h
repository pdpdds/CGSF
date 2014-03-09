#pragma once

namespace CGSF
{

typedef enum ePACKET_ID
{
 	Auth = 1000,
 	Login,
	LoginSuccess,
	LoginFail,
	TeamInfo,
	RoomMember,
	EnterTeamMember,
	LeaveTeamMember,
	InvalidPacket,
	EnterLobby,
	ChatReq,
	ChatRes,
	EnterRoom,
	CreateRoom,
	LeaveRoom,
	RoomList,
	RoomListRefresh,
	ChangeTeam,
	LoadingStart,
	LoadingComplete,
	PlayWait,
	PlayStart = 1021,
	PeerList,
	
	MSG_CREATE_PLAYER,
	MSG_DESTROY_PLAYER,	
	PlayEnd,
	PlayScoreInfo,
	PlayPlayerScore,
	PlayerIP,
	AddPeer,
	DeletePeer,
};
};
