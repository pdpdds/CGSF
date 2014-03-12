#pragma once
#include "BasePacket.h"
#include "SFFastCRC.h"

class SFCompress;

class SFPacket : public BasePacket
{
public:
	SFPacket(USHORT packetID);
	SFPacket();
	virtual ~SFPacket(void);

	BOOL Initialize();
	void ResetDataBuffer();

	static BOOL GetDataCRC(BYTE* pDataBuf, DWORD DataSize, DWORD& dwDataCRC);
	BOOL CheckDataCRC();

	inline int GetDataSize(){ return m_Header.dataSize; }
	inline void SetDataSize(USHORT size){ m_Header.dataSize = size; }
	
	inline int GetPacketSize(){ return sizeof(SFPacketHeader) + m_Header.dataSize; }
	
	inline BYTE* GetDataBuffer(){ return m_pPacketData; }
	SFPacketHeader* GetHeader(){return &m_Header;}

	inline USHORT GetHeaderSize() { return sizeof(SFPacketHeader); }

	virtual bool Encode() override;
	BOOL Decode(int& ErrorCode);
	//virtual bool Decode(char* pBuf, unsigned int nSize) override;

	inline void SetPacketID(unsigned short packetID){ BasePacket::SetPacketID(packetID);  m_Header.packetID = packetID; }

	void ReadStart(){ m_usCurrentReadPosition = 0; }
	void ReadEnd(){}

	inline SFPacket& SFPacket::operator << (BYTE Data)
	{
		memcpy(&m_pPacketData[m_Header.dataSize], &Data, sizeof(BYTE));
		m_Header.dataSize += sizeof(BYTE);

		SFASSERT(m_Header.dataSize <= MAX_PACKET_DATA);
		return *this;
	}

	inline SFPacket& SFPacket::operator << (CHAR Data)
	{
		memcpy(&m_pPacketData[m_Header.dataSize], &Data, sizeof(CHAR));
		m_Header.dataSize += sizeof(CHAR);

		SFASSERT(m_Header.dataSize <= MAX_PACKET_DATA);
		return *this;
	}

	inline SFPacket& SFPacket::operator << (SHORT Data)
	{
		memcpy(&m_pPacketData[m_Header.dataSize], &Data, sizeof(SHORT));
		m_Header.dataSize += sizeof(SHORT);

		SFASSERT(m_Header.dataSize <= MAX_PACKET_DATA);
		return *this;
	}

	inline SFPacket& SFPacket::operator << (USHORT Data)
	{
		memcpy(&m_pPacketData[m_Header.dataSize], &Data, sizeof(USHORT));
		m_Header.dataSize += sizeof(USHORT);

		SFASSERT(m_Header.dataSize <= MAX_PACKET_DATA);
		return *this;
	}

	inline SFPacket& SFPacket::operator << (int Data)
	{
		memcpy(&m_pPacketData[m_Header.dataSize], &Data, sizeof(int));
		m_Header.dataSize += sizeof(int);

		SFASSERT(m_Header.dataSize <= MAX_PACKET_DATA);
		return *this;
	}

	inline SFPacket& SFPacket::operator << (DWORD Data)
	{
		memcpy(&m_pPacketData[m_Header.dataSize], &Data, sizeof(DWORD));
		m_Header.dataSize += sizeof(DWORD);

		SFASSERT(m_Header.dataSize <= MAX_PACKET_DATA);
		return *this;
	}

	inline SFPacket& SFPacket::operator << (float Data)
	{
		memcpy(&m_pPacketData[m_Header.dataSize], &Data, sizeof(float));
		m_Header.dataSize += sizeof(float);

		SFASSERT(m_Header.dataSize <= MAX_PACKET_DATA);
		return *this;
	}

	inline SFPacket& SFPacket::operator << (char* szStr)
	{
		int len = strlen(szStr);
		SFASSERT(len > 0);

		memcpy(&m_pPacketData[m_Header.dataSize], szStr, len);
		m_Header.dataSize += len;
		m_pPacketData[m_Header.dataSize] = 0;
		m_Header.dataSize++;

		SFASSERT(m_Header.dataSize <= MAX_PACKET_DATA);
		return *this;
	}

	inline SFPacket& SFPacket::operator >> (BYTE& Data)
	{
		memcpy(&Data, &m_pPacketData[m_usCurrentReadPosition], sizeof(BYTE));
		m_usCurrentReadPosition += sizeof(BYTE);

		SFASSERT(m_usCurrentReadPosition <= MAX_PACKET_DATA);
		return *this;
	}

	inline SFPacket& SFPacket::operator >> (CHAR& Data)
	{
		memcpy(&Data, &m_pPacketData[m_usCurrentReadPosition], sizeof(CHAR));
		m_usCurrentReadPosition += sizeof(CHAR);

		SFASSERT(m_usCurrentReadPosition <= MAX_PACKET_DATA);
		return *this;
	}

	inline SFPacket& SFPacket::operator >> (SHORT& Data)
	{
		memcpy(&Data, &m_pPacketData[m_usCurrentReadPosition], sizeof(SHORT));
		m_usCurrentReadPosition += sizeof(SHORT);

		SFASSERT(m_usCurrentReadPosition <= MAX_PACKET_DATA);
		return *this;
	}

	inline SFPacket& SFPacket::operator >> (USHORT& Data)
	{
		memcpy(&Data, &m_pPacketData[m_usCurrentReadPosition], sizeof(USHORT));
		m_usCurrentReadPosition += sizeof(USHORT);

		SFASSERT(m_usCurrentReadPosition <= MAX_PACKET_DATA);
		return *this;
	}

	inline SFPacket& SFPacket::operator >> (int& Data)
	{
		memcpy(&Data, &m_pPacketData[m_usCurrentReadPosition], sizeof(int));
		m_usCurrentReadPosition += sizeof(int);

		SFASSERT(m_usCurrentReadPosition <= MAX_PACKET_DATA);
		return *this;
	}

	inline SFPacket& SFPacket::operator >> (DWORD& Data)
	{
		memcpy(&Data, &m_pPacketData[m_usCurrentReadPosition], sizeof(DWORD));
		m_usCurrentReadPosition += sizeof(DWORD);

		SFASSERT(m_usCurrentReadPosition <= MAX_PACKET_DATA);
		return *this;
	}

	inline SFPacket& SFPacket::operator >> (float& Data)
	{
		memcpy(&Data, &m_pPacketData[m_usCurrentReadPosition], sizeof(float));
		m_usCurrentReadPosition += sizeof(float);

		SFASSERT(m_usCurrentReadPosition <= MAX_PACKET_DATA);
		return *this;
	}

	inline SFPacket& SFPacket::operator >> (char* szStr)
	{
		strcpy_s(szStr, strlen((char*)&m_pPacketData[m_usCurrentReadPosition]) + 1, (char*)&m_pPacketData[m_usCurrentReadPosition]);
		m_usCurrentReadPosition += strlen((char*)&m_pPacketData[m_usCurrentReadPosition]) + 1;

		SFASSERT(m_usCurrentReadPosition <= MAX_PACKET_DATA);

		return *this;
	}

	void SFPacket::Read(BYTE* pBuffer, int BufferSize)
	{
		memcpy(pBuffer, &m_pPacketData[m_usCurrentReadPosition], BufferSize);
		m_usCurrentReadPosition += BufferSize;

		SFASSERT(m_usCurrentReadPosition <= MAX_PACKET_DATA);
	}

	void SFPacket::Write(const BYTE* pBuffer, int BufferSize)
	{
		memcpy(&m_pPacketData[m_Header.dataSize], pBuffer, BufferSize);
		m_Header.dataSize += BufferSize;

		SFASSERT(m_Header.dataSize <= MAX_PACKET_DATA);
	}

protected:

private:
	SFPacketHeader m_Header;
	BYTE m_pPacketData[MAX_PACKET_DATA];
	USHORT m_usCurrentReadPosition;

	static SFFastCRC m_FastCRC;
	bool m_bEncoded;
};