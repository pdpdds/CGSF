#pragma once

class SFPlayer;
class SFRoom;

BOOL SendToClient(SFPlayer* pPlayer, BasePacket* pPacket);

BOOL SendAuthPacket(int serial);

BOOL SendLoginResult(SFPlayer* pPlayer,int result);
BOOL SendEnterLobby(SFPlayer* pPlayer);

BOOL SendCreateRoom( SFPlayer* pPlayer, int GameMode );

BOOL SendEnterRoom( SFPlayer* pPlayer, int GameMode, int RoomIndex );
BOOL SendLeaveRoom(SFPlayer* pPlayer);

BOOL SendLoadingStart(SFPlayer* pPlayer);

BOOL SendPlayStart(SFPlayer* pPlayer);
BOOL SendPlayEnd(SFPlayer* pPlayer);

BOOL SendRoomMember(SFPlayer* pPlayer, SFRoom* pRoom);

BOOL SendEnterTeamMember(SFPlayer* pPlayer, int PlayerIndex, char* szPlayerName);
BOOL SendLeaveTeamMember(SFPlayer* pPlayer, int PlayerIndex, char* szPlayerName );

//////////////////////////////////////////////////////////////////////////////////
//Logic
//////////////////////////////////////////////////////////////////////////////////
BOOL SendCreatePlayer(SFPlayer* pPlayer, SFRoom* pRoom, BOOL exceptMe = FALSE);
BOOL SendDestroyPlayer( SFPlayer* pPlayer, int PlayerIndex);
BOOL SendPeerInfo( SFPlayer* pPlayer, SFRoom* pRoom);
BOOL SendDeletePeer(SFPlayer* pPlayer, int PlayerIndex);
