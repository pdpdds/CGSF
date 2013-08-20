#pragma once

class SFProactorService;
class SFPlayer;
class SFRoom;

BOOL SendToClient(SFPlayer* pPlayer, BasePacket* pPacket);

BOOL SendAuthPacket(int Serial);

BOOL SendLoginResult(SFPlayer* pPlayer,int Result);
BOOL SendEnterLobby(SFPlayer* pPlayer);

BOOL SendCreateRoom( SFPlayer* pPlayer, int GameMode );

BOOL SendEnterRoom( SFPlayer* pPlayer, int GameMode, int RoomIndex );
BOOL SendLeaveRoom(SFPlayer* pPlayer);


BOOL SendLoadingStart(SFPlayer* pPlayer);

BOOL SendPlayStart(SFPlayer* pPlayer);
BOOL SendPlayEnd(SFPlayer* pPlayer);

BOOL SendEnterTeamMember(SFPlayer* pPlayer, int PlayerIndex);
BOOL SendLeaveTeamMember(SFPlayer* pPlayer, int PlayerIndex);

//////////////////////////////////////////////////////////////////////////////////
//Logic
//////////////////////////////////////////////////////////////////////////////////
BOOL SendCreatePlayer(SFPlayer* pPlayer, SFRoom* pRoom, BOOL ExceptMe = FALSE);
BOOL SendSpawnPlayer( SFPlayer* pPlayer,SFPlayer* pTarget);
BOOL SendDestroyPlayer( SFPlayer* pPlayer, int PlayerIndex);
BOOL SendPlayerHealth( SFPlayer* pPlayer,SFPlayer* pTarget);
BOOL SendPeerInfo( SFPlayer* pPlayer, SFRoom* pRoom);
BOOL SendDeletePeer(SFPlayer* pPlayer, int PlayerIndex);
