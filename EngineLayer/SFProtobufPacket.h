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

protected:

private:
	T m_Packet;
};

