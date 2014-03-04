#pragma once
#include "BasePacket.h"
#include "SFFastCRC.h"

class SFCompress;

class SFPacket : public BasePacket
{
public:
	SFPacket(void);
	virtual ~SFPacket(void);

	BOOL Initialize();
	void ResetDataBuffer();

	static BOOL GetDataCRC(BYTE* pDataBuf, DWORD DataSize, DWORD& dwDataCRC);
	BOOL CheckDataCRC();

	inline int GetDataSize(){return m_Header.PacketLen - sizeof(SFPacketHeader);}
	inline void SetDataSize(USHORT Size){m_Header.PacketLen = Size + sizeof(SFPacketHeader);}
	inline BYTE* GetDataBuffer() {return m_pPacketData;}

	inline USHORT GetHeaderSize() {return sizeof(SFPacketHeader);}
	inline BYTE* GetPacketData(){return m_pPacketData;}
	SFPacketHeader* GetHeader(){return &m_Header;}

	inline int GetPacketSize(){return m_Header.PacketLen;}

	virtual bool Encode() override;
	BOOL Decode(int& ErrorCode);
	//virtual bool Decode(char* pBuf, unsigned int nSize) override;

protected:

private:
	SFPacketHeader m_Header;
	BYTE m_pPacketData[MAX_PACKET_DATA];

	static SFFastCRC m_FastCRC;
};