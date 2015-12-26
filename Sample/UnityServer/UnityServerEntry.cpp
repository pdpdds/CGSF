#include "stdafx.h"
#include "UnityServerEntry.h"
#include "SFJsonPacket.h"
#include "GameUser.h"
#include <iostream>

#include "CommonProtocol.h"

#include "LoginPart.h"

#define CHAT_PACKET_NUM 1000

UnityServerEntry::UnityServerEntry()
{
}
UnityServerEntry::~UnityServerEntry()
{
}
bool UnityServerEntry::Initialize()
{
	return true;
}
bool UnityServerEntry::ProcessPacket(BasePacket* pBasePacket)
{

	switch (pBasePacket->GetPacketType())
	{
	case SFPACKET_DATA:
		switch (pBasePacket->GetPacketID())
		{
		case CommonProtocol::REQ_LOGIN:
			m_Login.REQD_LOGIN(pBasePacket);
			break;
		case CommonProtocol::KEEPALIVE:
			break;
		case CommonProtocol::REQ_IDISIDVALID:
			m_Login.REQD_IDISIDVALID(pBasePacket);
			break;
		case CommonProtocol::REQ_REGISTRATION:
			m_Login.REQD_REGISTRATION(pBasePacket);
			break;


		case CommonProtocol::REQ_ECHO:
			SFEngine::GetInstance()->SendRequest(pBasePacket);
			break;
		default:
			break;
		}
		break;
	case SFPACKET_CONNECT://立加 贸府@
		OnConnectPlayer(pBasePacket->GetSerial());
		break;
	case SFPACKET_DISCONNECT://昏力 贸府@
		OnDisconnectPlayer(pBasePacket->GetSerial());
		break;
	}

	return true;
}
bool UnityServerEntry::Broadcast(BasePacket* pPacket)
{
	for (auto& chatUser : m_ChatUserMap)
	{
		pPacket->SetSerial(chatUser.first);
		SFEngine::GetInstance()->SendRequest(pPacket);
	}

	return true;
}
GameUser* UnityServerEntry::FindUser(int serial)
{
	auto& iter = m_ChatUserMap.find(serial);
	if (iter == m_ChatUserMap.end())
	{
		return NULL;
	}
	return iter->second;
}
bool UnityServerEntry::OnConnectPlayer(int serial)
{
	GameUser* pUser = new GameUser();
	pUser->SetSerial(serial);
	pUser->SetName(serial);
	
	m_ChatUserMap.insert(DicUserMap::value_type(serial, pUser));

	return true;
}
bool UnityServerEntry::OnPlayerData(BasePacket* pPacket)
{
	SFJsonPacket* pJsonPacket = (SFJsonPacket*)pPacket;

	switch (pPacket->GetPacketID())
	{
	case CHAT_PACKET_NUM:
		GameUser* pUser = FindUser(pPacket->GetSerial());

		if (pUser == NULL)
			return false;

		std::cout << pUser->GetName().c_str() << " : " << pJsonPacket->GetData().GetValue<tstring>("chat") << std::endl;

		SFJsonPacket JsonPacket(CHAT_PACKET_NUM);
		JsonObjectNode& ObjectNode = JsonPacket.GetData();
		ObjectNode.Add("who", pUser->GetName());
		ObjectNode.Add("chat", pJsonPacket->GetData().GetValue<tstring>("chat"));

		Broadcast(&JsonPacket);

		break;
	}

	return true;
}
bool UnityServerEntry::OnDisconnectPlayer(int serial)
{
	auto& iter = m_ChatUserMap.find(serial);
	if (iter == m_ChatUserMap.end())
	{
		return false;
	}
	GameUser* pUser = iter->second;
	delete pUser;

	m_ChatUserMap.erase(iter);
	return true;
}
