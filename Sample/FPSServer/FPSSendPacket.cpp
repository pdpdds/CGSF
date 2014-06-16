#include "stdafx.h"
#include "FPSSendPacket.h"
#include "SFGameConstant.h"
#include "GamePacketStructure.h"
#include "SFProtobufPacket.h"
#include "SFPlayer.h"
#include "SFEngine.h"

BOOL SendSpawnPlayer(SFPlayer* pPlayer, SFPlayer* pTarget)
{
	SFProtobufPacket<FPSPacket::MSG_SPAWN_PLAYER> PktMsgSpawnPlayer = SFProtobufPacket<FPSPacket::MSG_SPAWN_PLAYER>(FPS::MSG_SPAWN_PLAYER);

	_CharacterInfo* pInfo = pTarget->GetCharacterInfo();

	SpawnPlayerMsg msg;
	PktMsgSpawnPlayer.SetSerial(pPlayer->GetSerial());
	msg.PlayerID = pTarget->GetSerial();
	msg.translation = pInfo->translation;

	PktMsgSpawnPlayer.GetData().set_spawnplayer(&msg, sizeof(SpawnPlayerMsg));

	SFEngine::GetInstance()->SendRequest(&PktMsgSpawnPlayer);
	return TRUE;
}

BOOL SendPlayerHealth(SFPlayer* pPlayer, SFPlayer* pTarget)
{
	SFProtobufPacket<FPSPacket::MSG_PLAYER_HEALTH> PktPlayerHealth = SFProtobufPacket<FPSPacket::MSG_PLAYER_HEALTH>(FPS::MSG_PLAYER_HEALTH);
	PktPlayerHealth.SetSerial(pPlayer->GetSerial());

	_CharacterInfo* pInfo = pTarget->GetCharacterInfo();

	PlayerHealthMsg msg;

	msg.PlayerID = pTarget->GetSerial();
	msg.health = pInfo->health;

	PktPlayerHealth.GetData().set_playerhealth(&msg, sizeof(SpawnPlayerMsg));

	SFEngine::GetInstance()->SendRequest(&PktPlayerHealth);

	return TRUE;
}