#pragma once

#define SFPACKET_NONE			0x00000001
#define SFPACKET_CONNECT		0x00000002
#define SFPACKET_DATA			0x00000004
#define SFPACKET_RPC			0x00000008
#define SFPACKET_TIMER			0x00000010
#define SFPACKET_SHOUTER		0x00000020
#define SFPACKET_DISCONNECT		0x00000040
#define SFPACKET_DB				0x00000080
#define SFPACKET_SERVERSHUTDOWN	0x00000100
#define SFPACKET_AUTHENTICATE	0x00000200

class BasePacket
{
public:
	BasePacket(){ m_serial = -1; m_packetType = (unsigned int)SFPACKET_NONE; m_packetID = (unsigned short)-1; }
	virtual ~BasePacket() {}

	void SetPacketType(unsigned int PacketType){ m_packetType = PacketType; }
	unsigned int GetPacketType(){ return m_packetType; }

	inline unsigned short GetPacketID(){return m_packetID;}
	inline void SetPacketID(unsigned short packetID){m_packetID = packetID;}
	
	void SetSerial(int serial){ m_serial = serial; }
	int GetSerial(){ return m_serial; }	

	void SetSessionDesc(_SessionDesc& sessiondesc)
	{ 
		m_sessiondesc.identifier = sessiondesc.identifier;
		m_sessiondesc.sessionType = sessiondesc.sessionType;
		m_sessiondesc.port = sessiondesc.port;
		m_sessiondesc.szIP = sessiondesc.szIP;
		m_sessiondesc.pSession = sessiondesc.pSession;
	}

	_SessionDesc& GetSessionDesc(){ return m_sessiondesc; }

	void CopyBaseHeader(BasePacket* pSource)
	{
		SetSerial(pSource->GetSerial());
		SetPacketID(pSource->GetPacketID());
		SetPacketType(pSource->GetPacketType());
	}

	virtual BasePacket* Clone(){ return NULL; }

	virtual bool Encode(unsigned short packetSize, int packetOption) { UNREFERENCED_PARAMETER(packetSize); UNREFERENCED_PARAMETER(packetOption);  return true; }
	virtual bool Decode(char* pBuf, unsigned int nSize) { UNREFERENCED_PARAMETER(pBuf); UNREFERENCED_PARAMETER(nSize); return true; }
	virtual void Release() {};

protected:

private:
	int m_serial;
	_SessionDesc m_sessiondesc;
	unsigned int m_packetType;
	unsigned short m_packetID;	
};