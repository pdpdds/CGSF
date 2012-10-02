#pragma once

namespace CGSF
{

typedef enum ePacketID
{
 	Auth = 1000,
 	Login,
	LoginSuccess,
	LoginFail,
	TeamInfo,
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
	ChangeTeam,
	StartGame,
	LoadingComplete,
	PlayWait,
	PlayStart,
	PeerList,
	
	MSG_CREATE_PLAYER, //게임 시작시 처음 한번만 또는 난입 한번만...
	MSG_DESTROY_PLAYER,
	MSG_PLAYER_HEALTH,
	MSG_PLAYER_SCORE,
	MSG_PLAYER_WEAPON_CHANGE,
	MSG_PLAYER_WEAPON_CHANGING,
	MSG_SPAWN_PLAYER,

	MSG_PLAYER_MOVE_UPDATE, //이 두개는 UDP로
	MSG_PLAYER_LOOK_UPDATE, //이 두개는 UDP로

	PlayEnd,
	PlayScoreInfo,
	PlayPlayerScore,
	PlayerIP,
	AddPeer,
	DeletePeer,
};
};
