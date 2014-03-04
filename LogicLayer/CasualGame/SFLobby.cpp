#include "StdAfx.h"
#include "SFLobby.h"
#include "SFPlayer.h"
#include "SFSendPacket.h"

SFLobby::SFLobby(void)
{
}

SFLobby::~SFLobby(void)
{
}

void SFLobby::OnEnter( SFPlayer* pPlayer )
{
	m_PlayerMap.insert(std::make_pair(pPlayer->GetSerial(), pPlayer));
}

void SFLobby::OnLeave( SFPlayer* pPlayer )
{
	m_PlayerMap.erase(pPlayer->GetSerial());
}

BOOL SFLobby::OnChat( SFPlayer* pPlayer, const std::string& szMessage )
{
	SFPacketStore::ChatRes PktChatRes;
	PktChatRes.set_sender(pPlayer->m_username);
	PktChatRes.set_message(szMessage);

	int BufSize = PktChatRes.ByteSize();
	
	PlayerMap::iterator iter = m_PlayerMap.begin();

	for(;iter != m_PlayerMap.end(); iter++)
	{
		SFPlayer* pTarget = iter->second;

		if(pTarget == pPlayer)
			continue;

		//SendToClient(pTarget, CGSF::ChatRes, &PktChatRes, BufSize);
	}

	return TRUE;
}