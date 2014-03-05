#pragma once
#include <json/jsonnode.h>

class SFJsonPacket : public BasePacket
{
public:
	SFJsonPacket();
	~SFJsonPacket(void);

	JsonObjectNode&	GetData(){return m_Node;}

protected:

private:
	JsonObjectNode m_Node;
};

