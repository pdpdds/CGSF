#pragma once
#include <memory>
#include "IOBuffer.h"
#include "SFPacketHeader.h"

#define PACKETIO_SIZE					8096
#define PACKETIO_ERROR_NONE				0
#define PACKETIO_ERROR_HEADER			-1
#define PACKETIO_ERROR_DATA				-2
#define PACKETIO_ERROR_DATA_CRC			-3
#define PACKETIO_ERROR_DATA_COMPRESS	-4
#define PACKETIO_ERROR_DATA_ENCRYPTION	-5

#define MAX_PACKET_DATA 2048

class SFPacket
{
public:
	SFPacket(unsigned short packetID);
	SFPacket();
	virtual ~SFPacket(void);

	bool Initialize();
	void ResetDataBuffer();	

	inline int GetDataSize(){ return m_Header.dataSize; }
	inline void SetDataSize(unsigned short size){ m_Header.dataSize = size; }
	
	inline int GetPacketSize(){ return sizeof(SFPacketHeader) + m_Header.dataSize; }
	
	inline unsigned char* GetDataBuffer(){ return m_pPacketData; }
	SFPacketHeader* GetHeader(){return &m_Header;}

	inline unsigned short GetHeaderSize() { return sizeof(SFPacketHeader); }
	

	inline void SetPacketID(unsigned short packetID){ m_Header.packetID = packetID; }

	void ReadStart(){ m_usCurrentReadPosition = 0; }
	void ReadEnd(){}

	inline SFPacket& SFPacket::operator << (unsigned char Data)
	{
		memcpy(&m_pPacketData[m_Header.dataSize], &Data, sizeof(unsigned char));
		m_Header.dataSize += sizeof(unsigned char);

		return *this;
	}

	inline SFPacket& SFPacket::operator << (char Data)
	{
		memcpy(&m_pPacketData[m_Header.dataSize], &Data, sizeof(char));
		m_Header.dataSize += sizeof(char);
		
		return *this;
	}

	inline SFPacket& SFPacket::operator << (short Data)
	{
		memcpy(&m_pPacketData[m_Header.dataSize], &Data, sizeof(short));
		m_Header.dataSize += sizeof(short);

		return *this;
	}

	inline SFPacket& SFPacket::operator << (unsigned short Data)
	{
		memcpy(&m_pPacketData[m_Header.dataSize], &Data, sizeof(unsigned short));
		m_Header.dataSize += sizeof(unsigned short);

		return *this;
	}

	inline SFPacket& SFPacket::operator << (int Data)
	{
		memcpy(&m_pPacketData[m_Header.dataSize], &Data, sizeof(int));
		m_Header.dataSize += sizeof(int);

		return *this;
	}

	inline SFPacket& SFPacket::operator << (unsigned int Data)
	{
		memcpy(&m_pPacketData[m_Header.dataSize], &Data, sizeof(unsigned int));
		m_Header.dataSize += sizeof(unsigned int);

		return *this;
	}

	inline SFPacket& SFPacket::operator << (float Data)
	{
		memcpy(&m_pPacketData[m_Header.dataSize], &Data, sizeof(float));
		m_Header.dataSize += sizeof(float);

		return *this;
	}

	inline SFPacket& SFPacket::operator << (char* szStr)
	{
		int len = strlen(szStr);

		memcpy(&m_pPacketData[m_Header.dataSize], szStr, len);
		m_Header.dataSize += len;
		m_pPacketData[m_Header.dataSize] = 0;
		m_Header.dataSize++;
		
		return *this;
	}

	inline SFPacket& SFPacket::operator >> (unsigned char& Data)
	{
		memcpy(&Data, &m_pPacketData[m_usCurrentReadPosition], sizeof(unsigned char));
		m_usCurrentReadPosition += sizeof(unsigned char);
		
		return *this;
	}

	inline SFPacket& SFPacket::operator >> (char& Data)
	{
		memcpy(&Data, &m_pPacketData[m_usCurrentReadPosition], sizeof(char));
		m_usCurrentReadPosition += sizeof(char);
		
		return *this;
	}

	inline SFPacket& SFPacket::operator >> (short& Data)
	{
		memcpy(&Data, &m_pPacketData[m_usCurrentReadPosition], sizeof(short));
		m_usCurrentReadPosition += sizeof(short);

		return *this;
	}

	inline SFPacket& SFPacket::operator >> (unsigned short& Data)
	{
		memcpy(&Data, &m_pPacketData[m_usCurrentReadPosition], sizeof(unsigned short));
		m_usCurrentReadPosition += sizeof(unsigned short);

		return *this;
	}

	inline SFPacket& SFPacket::operator >> (int& Data)
	{
		memcpy(&Data, &m_pPacketData[m_usCurrentReadPosition], sizeof(int));
		m_usCurrentReadPosition += sizeof(int);

		return *this;
	}

	inline SFPacket& SFPacket::operator >> (unsigned int& Data)
	{
		memcpy(&Data, &m_pPacketData[m_usCurrentReadPosition], sizeof(unsigned int));
		m_usCurrentReadPosition += sizeof(unsigned int);

		return *this;
	}

	inline SFPacket& SFPacket::operator >> (float& Data)
	{
		memcpy(&Data, &m_pPacketData[m_usCurrentReadPosition], sizeof(float));
		m_usCurrentReadPosition += sizeof(float);

		return *this;
	}

	inline SFPacket& SFPacket::operator >> (char* szStr)
	{
		strcpy_s(szStr, strlen((char*)&m_pPacketData[m_usCurrentReadPosition]) + 1, (char*)&m_pPacketData[m_usCurrentReadPosition]);
		m_usCurrentReadPosition += strlen((char*)&m_pPacketData[m_usCurrentReadPosition]) + 1;

		return *this;
	}

	void SFPacket::Read(unsigned char* pBuffer, int BufferSize)
	{
		memcpy(pBuffer, &m_pPacketData[m_usCurrentReadPosition], BufferSize);
		m_usCurrentReadPosition += BufferSize;

	}

	void SFPacket::Write(const unsigned char* pBuffer, int BufferSize)
	{
		memcpy(&m_pPacketData[m_Header.dataSize], pBuffer, BufferSize);
		m_Header.dataSize += BufferSize;

	}

protected:

private:
	SFPacketHeader m_Header;
	unsigned char m_pPacketData[MAX_PACKET_DATA];
	unsigned short m_usCurrentReadPosition;
};