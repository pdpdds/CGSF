#include "stdafx.h"
#include "EchoLogicEntry.h"
#include "SFJsonPacket.h"
#include "GameUser.h"

EchoLogicEntry::EchoLogicEntry(void)
{
}


EchoLogicEntry::~EchoLogicEntry(void)
{
}

bool EchoLogicEntry::Initialize()
{
	return true;
}

bool EchoLogicEntry::ProcessPacket(BasePacket* pPacket )
{
	switch (pPacket->GetPacketType())	
	{	
	case SFPACKET_CONNECT:
		printf("connect user %d!!\n", pPacket->GetSerial());
		return OnConnect(pPacket);

	case SFPACKET_DISCONNECT:
		printf("disconnect user %d!!\n", pPacket->GetSerial());
		return OnDisconnect(pPacket);	
	case SFPACKET_DATA:	
		//SFEngine::GetInstance()->SendRequest(pPacket);
		Broadcast(pPacket);
		break;
	}

	return true;
}

bool EchoLogicEntry::OnConnect(BasePacket* pPacket)
{
	GameUser* pUser = new GameUser();
	pUser->SetSerial(pPacket->GetSerial());

	m_GameUserMap.insert(GameUserMap::value_type(pPacket->GetSerial(), pUser));
	return true;
}

bool EchoLogicEntry::OnDisconnect(BasePacket* pPacket)
{
	auto& iter = m_GameUserMap.find(pPacket->GetSerial());

	if (iter == m_GameUserMap.end())
	{
		SFASSERT(0);
		return false;
	}

	GameUser* pUser = iter->second;

	delete pUser;
	return true;
}

bool EchoLogicEntry::Broadcast(BasePacket* pPacket)
{
	for (auto& chatUser : m_GameUserMap)
	{
		pPacket->SetSerial(chatUser.first);
		SFEngine::GetInstance()->SendRequest(pPacket);
	}

	return true;
}

GameUser* EchoLogicEntry::FindUser(int serial)
{
	auto& iter = m_GameUserMap.find(serial);

	if (iter == m_GameUserMap.end())
	{
		return NULL;
	}

	return iter->second;
}


