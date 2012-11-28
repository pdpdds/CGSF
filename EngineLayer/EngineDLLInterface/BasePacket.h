#pragma once

enum eSFPacketType
{
	SFPacket_None = 0,
	SFPacket_Connect,
	SFPacket_Data,
	SFPacket_Timer,
	SFPacket_Shouter,
	SFPacket_Disconnect,
	SFPacket_DB,
};

class BasePacket
{
public:
	BasePacket(){m_PacketOwnerSerial = -1; m_PacketType = SFPacket_None; m_PacketID = -1;}
	~BasePacket(){}

	void SetPacketType(eSFPacketType PacketType){m_PacketType = PacketType;}
	eSFPacketType GetPacketType(){return m_PacketType;}

	inline unsigned short GetPacketID(){return m_PacketID;}
	inline void SetPacketID(unsigned short PacketID){m_PacketID = PacketID;}
	
	void SetOwnerSerial(int Serial){m_PacketOwnerSerial = Serial;}
	int GetOwnerSerial(){return m_PacketOwnerSerial;}

	virtual bool Encode() {return false;}
	virtual bool Decode(char* pBuf, unsigned int nSize) {return false;}
	

protected:

private:
	int m_PacketOwnerSerial;
	eSFPacketType m_PacketType;
	unsigned short m_PacketID;
};