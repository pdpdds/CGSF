#pragma once
#include "ILogicEntry.h"
#include "SFDispatch.h"

class BasePacket;

template <typename T>
class ProtocolLogicEntry : public ILogicEntry
{
public:
	ProtocolLogicEntry(void){}
	virtual ~ProtocolLogicEntry(void){}

	virtual bool Initialize() override;
	virtual bool ProcessPacket(BasePacket* pBasePacket) override;

protected:	

private:
	T m_PacketHandler;	
	SFDispatch<USHORT, std::function<bool(BasePacket*)>, BasePacket*> m_Dispatch;
};

template<typename T>
bool ProtocolLogicEntry<T>::Initialize()
{
	m_Dispatch.RegisterMessage(Protocol::Sample1, std::bind(&T::OnPacketSample1, &(this->m_PacketHandler), std::tr1::placeholders::_1));
	m_Dispatch.RegisterMessage(Protocol::Sample2, std::bind(&T::OnPacketSample2, &(this->m_PacketHandler), std::tr1::placeholders::_1));
	m_Dispatch.RegisterMessage(Protocol::Sample3, std::bind(&T::OnPacketSample3, &(this->m_PacketHandler), std::tr1::placeholders::_1));
	m_Dispatch.RegisterMessage(Protocol::Sample4, std::bind(&T::OnPacketSample4, &(this->m_PacketHandler), std::tr1::placeholders::_1));

	return true;
}

template<typename T>
bool ProtocolLogicEntry<T>::ProcessPacket(BasePacket* pPacket)
{
	switch (pPacket->GetPacketType())
	{
	case SFPACKET_DATA:
		return m_Dispatch.HandleMessage(pPacket->GetPacketID(), pPacket);
	case SFPACKET_CONNECT:
		printf("new user coming!!\n");
	}

	return true;
}