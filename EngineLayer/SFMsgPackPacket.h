#pragma once
#include <msgpack.hpp>
#include "BasePacket.h"

class SFMsgPackPacket : public BasePacket
{
	friend class SFMsgPackProtocol;

public:
	SFMsgPackPacket(USHORT usPacketId);
	virtual ~SFMsgPackPacket();

	SFPacketHeader* GetHeader() { return &m_packetHeader; }
	msgpack::unpacker&	GetData(){ return m_unpacker; }

protected:

private:

	SFMsgPackPacket();

	SFPacketHeader m_packetHeader;
	msgpack::unpacker m_unpacker;
};