#pragma once
#include <json/jsonnode.h>
#include "BasePacket.h"

class SFJsonPacket : public BasePacket
{
	friend class SFJsonProtocol;

public:
	SFJsonPacket(USHORT usPacketId);
	~SFJsonPacket(void);

	JsonObjectNode&	GetData(){return m_Node;}
	SFPacketHeader* GetHeader() { return &m_Header; }
protected:

private:
	SFJsonPacket();
	JsonObjectNode m_Node;

	SFPacketHeader m_Header;
};