#pragma once
#include "SFProtobufPacketImpl.h"

template<class T>
class SFProtobufPacket : public SFProtobufPacketImpl
{
public:

	SFProtobufPacket(int serviceId)
		: SFProtobufPacketImpl(serviceId, &m_Packet)
	{
	}
		
	virtual ~SFProtobufPacket(void){}

	T& GetData()
	{
		return m_Packet;
	}
	
	virtual BasePacket* Clone()
	{
		SFProtobufPacket* pClone = new SFProtobufPacket<T>(getServiceId());
		pClone->CopyBaseHeader(this);
		pClone->m_Packet = m_Packet;

		return pClone;
	}


protected:

private:
	T m_Packet;
};

