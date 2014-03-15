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

protected:

private:
	SFJsonPacket();
	JsonObjectNode m_Node;
};