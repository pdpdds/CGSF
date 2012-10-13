#include "StdAfx.h"
#include "SFPlayerLogin.h"
#include "SFPlayer.h"
#include "SFDataBaseProxy.h"
#include "SFPacketStore.pb.h"
#include "PacketID.h"
#include "DBMsg.h"

SFPlayerLogin::SFPlayerLogin(SFPlayer* pOwner, ePlayerState State)
: SFPlayerState(pOwner, State)
{
}

SFPlayerLogin::~SFPlayerLogin(void)
{
}

BOOL SFPlayerLogin::OnEnter()
{
	return TRUE;
}

BOOL SFPlayerLogin::OnLeave()
{
	return TRUE;
}

BOOL SFPlayerLogin::ProcessPacket( SFPacket* pPacket )
{
	switch(pPacket->GetPacketID())
	{
	case CGSF::EnterLobby:
		{
			OnEnterLobby(pPacket);
		}
		break;
	default:
		return FALSE;
	}

	return TRUE;
}

BOOL SFPlayerLogin::ProcessDBResult( SFMessage* pMessage )
{
	return FALSE;
}

BOOL SFPlayerLogin::OnEnterLobby( SFPacket* pPacket )
{
	SFPacketStore::EnterLobby PktEnterLobby;
	protobuf::io::ArrayInputStream is(pPacket->GetDataBuffer(), pPacket->GetDataSize());
	PktEnterLobby.ParseFromZeroCopyStream(&is);

	SFPlayer* pPlayer = GetOwner();

	pPlayer->ChangeState(PLAYER_STATE_LOBBY);

	return TRUE;
}