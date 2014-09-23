#include "stdafx.h"
#include "ServerLogicEntry.h"
#include "CgsfNET64.h"

namespace CgsfNET64Lib {

	bool ServerLogicEntry::Initialize()
	{
		return true;
	}

	bool ServerLogicEntry::SendPacket(BasePacket* pPacket)
	{
		auto result = SFEngine::GetInstance()->SendRequest(pPacket);
		return result;
	}

	bool ServerLogicEntry::ProcessPacket(BasePacket* pPacket)
	{
		//LOG(INFO) << "ServerLogicEntry::ProcessPacket. PacketType: " << pPacket->GetPacketType();
		//google::FlushLogFiles(google::GLOG_INFO);

		m_refPacketQueue->Enqueue(pPacket);

		return true;
	}
		
	//bool ServerLogicEntry::OnConnect(int serial)
	//{
	//	//ChatUser* pUser = new ChatUser();
	//	//pUser->SetSerial(serial);
	//	//pUser->SetName(serial);

	//	//m_ChatUserMap.insert(ChatUserMap::value_type(serial, pUser));

	//	return true;
	//}

	//bool ServerLogicEntry::OnDisconnect(int serial)
	//{
	//	auto& iter = m_ChatUserMap.find(serial);

	//	if (iter == m_ChatUserMap.end())
	//	{
	//		SFASSERT(0);
	//		return false;
	//	}

	//	ChatUser* pUser = iter->second;

	//	delete pUser;

	//	m_ChatUserMap.erase(iter);

	//	return true;
	//}

	/*bool ServerLogicEntry::OnProcessData(BasePacket* pPacket)
	{
		SFJsonPacket* pJsonPacket = (SFJsonPacket*)pPacket;

		switch (pPacket->GetPacketID())
		{
		case CHAT_PACKET_NUM:
			ChatUser* pUser = FindUser(pPacket->GetSerial());

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
	}*/
}