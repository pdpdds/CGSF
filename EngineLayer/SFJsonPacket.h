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
	SFPacketHeader* GetHeader() { return &m_packetHeader; }
	virtual void Release() override;

	virtual BasePacket* Clone() override;

protected:

private:
	SFJsonPacket();

	SFPacketHeader m_packetHeader;
	JsonObjectNode m_Node;	
};