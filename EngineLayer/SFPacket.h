#pragma once
#include "SFFastCRC.h"

class SFCompress;

enum eSFCommand
{
	SFCommand_None = 0,
	SFCommand_Connect,
	SFCommand_Data,
	SFCommand_Timer,
	SFCommand_Shouter,
	SFCommand_Disconnect,
	SFCommand_DB,
};

class SFCommand
{
public:
	SFCommand(){m_PacketOwnerSerial = -1; m_CommandType = SFCommand_Connect;}
	~SFCommand(){}

	void SetPacketType(eSFCommand PacketType){m_CommandType = PacketType;}
	eSFCommand GetPacketType(){return m_CommandType;}
	
	void SetOwnerSerial(int Serial){m_PacketOwnerSerial = Serial;}
	int GetOwnerSerial(){return m_PacketOwnerSerial;}

protected:

private:
	int m_PacketOwnerSerial;
	eSFCommand m_CommandType;
};

class SFPacket : public SFCommand
{
public:
	SFPacket(void);
	virtual ~SFPacket(void);

	BOOL Initialize();
	void ResetDataBuffer();

	BOOL MakePacket(BYTE* pSrcBuf, int SrcSize, int PacketOption = 0);
	static BOOL GetDataCRC(BYTE* pDataBuf, DWORD DataSize, DWORD& dwDataCRC);
	
	inline USHORT GetPacketID(){return m_Header.PacketID;}
	inline void SetPacketID(USHORT PacketID){m_Header.PacketID = PacketID;}

	inline int GetDataSize(){return m_Header.PacketLen - sizeof(SFPacketHeader);}
	inline void SetDataSize(USHORT Size){m_Header.PacketLen = Size + sizeof(SFPacketHeader);}
	inline BYTE* GetDataBuffer() {return m_pPacketData;}

	inline USHORT GetHeaderSize() {return sizeof(SFPacketHeader);}
	inline BYTE* GetPacketData(){return m_pPacketData;}
	SFPacketHeader* GetHeader(){return &m_Header;}

	inline int GetPacketSize(){return m_Header.PacketLen;}

protected:

private:
	SFPacketHeader m_Header;
	BYTE m_pPacketData[MAX_PACKET_DATA];

	static SFFastCRC m_FastCRC;
};