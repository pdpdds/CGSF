#pragma once
#include "SFDataBaseProxy.h"

class BasePacket;

template<typename T>
class SFDBPacketSystem
{
public:	
	static SFDBPacketSystem* GetInstance()
	{
		if (m_pDBPacketSystem == 0)
			m_pDBPacketSystem = new SFDBPacketSystem();

		return m_pDBPacketSystem;
	}

	T* GetInitMessage(int RequestMsg, DWORD PlayerSerial)
	{
		T* pMessage = AllocDBMsg();

		SFASSERT(pMessage != NULL);
		pMessage->Initialize(RequestMsg);
		pMessage->SetPacketID(RequestMsg);
		pMessage->SetSerial(PlayerSerial);
		pMessage->SetPacketType(SFPACKET_DB);

		return pMessage;
	}

	BOOL SendDBRequest(BasePacket* pMessage)
	{
		return SFLogicEntry::GetLogicEntry()->GetDataBaseProxy()->SendDBRequest(pMessage);
	}

	void SendToLogic(BasePacket* pMessage)
	{
		pMessage->SetPacketType(SFPACKET_DB);
		LogicGatewaySingleton::instance()->PushPacket(pMessage);
	}

	T* AllocDBMsg()
	{
		return m_DBMessagePool.Alloc();
	}

	bool RecallDBMsg(BasePacket* pMessage)
	{
		return m_DBMessagePool.Release((T*)pMessage);
	}


private:
	SFDBPacketSystem()
	{
	}

	virtual ~SFDBPacketSystem()
	{}

	static SFDBPacketSystem* m_pDBPacketSystem;

private:
	static SFObjectPool<T> m_DBMessagePool;
};

template<typename T>
SFObjectPool<T> SFDBPacketSystem<T>::m_DBMessagePool;

template<typename T>
SFDBPacketSystem<T>* SFDBPacketSystem<T>::m_pDBPacketSystem = 0;