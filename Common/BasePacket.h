#pragma once


enum eSFPacketType
{
	SFPACKET_NONE = 0,
	SFPACKET_CONNECT,
	SFPACKET_DATA,
	SFPACKET_RPC,
	SFPACKET_TIMER,
	SFPACKET_SHOUTER,
	SFPACKET_DISCONNECT,
	SFPACKET_DB,
	SFPACKET_SERVERSHUTDOWN,
};

class BasePacket
{
public:
	BasePacket(){ m_PacketOwnerSerial = -1; m_PacketType = SFPACKET_NONE; m_PacketID = -1; }
	virtual ~BasePacket() {}

	void SetPacketType(eSFPacketType PacketType){m_PacketType = PacketType;}
	eSFPacketType GetPacketType(){return m_PacketType;}

	inline unsigned short GetPacketID(){return m_PacketID;}
	inline void SetPacketID(unsigned short PacketID){m_PacketID = PacketID;}
	
	void SetOwnerSerial(int Serial){m_PacketOwnerSerial = Serial;}
	int GetOwnerSerial(){return m_PacketOwnerSerial;}

	void CopyBaseHeader(BasePacket* pSource)
	{
		SetOwnerSerial(pSource->GetOwnerSerial());
		SetPacketID(pSource->GetPacketID());
		SetPacketType(pSource->GetPacketType());
	}

	virtual BasePacket* Clone(){ return NULL; }

	virtual bool Encode() { return true; }
	virtual bool Decode(char* pBuf, unsigned int nSize) {return true;}

protected:

private:
	int m_PacketOwnerSerial;
	eSFPacketType m_PacketType;
	unsigned short m_PacketID;
};