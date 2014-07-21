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
	void ResetBuffer();
	void ResetDataBuffer();

	virtual void Release() override;
	static BOOL GetDataCRC(BYTE* pDataBuf, DWORD dwDataSize, DWORD& dwDataCRC);
	BOOL CheckDataCRC();

	inline int GetDataSize(){ return m_pHeader->dataSize; }
	inline void SetDataSize(USHORT size){ m_pHeader->dataSize = size; }
	
	inline int GetPacketSize(){ return sizeof(SFPacketHeader) + m_pHeader->dataSize; }
	
	SFPacketHeader* GetHeader(){ return m_pHeader; }
	inline BYTE* GetData(){ return m_pPacketBuffer + sizeof(SFPacketHeader); }	

	virtual bool Encode(unsigned short packetSize, int packetOption) override;
	bool Decode(unsigned short packetSize, int& ErrorCode);
	//virtual bool Decode(char* pBuf, unsigned int nSize) override;

	inline void SetPacketID(unsigned short packetID){ BasePacket::SetPacketID(packetID);  m_pHeader->packetID = packetID; }

	void ReadStart(){ m_usCurrentReadPosition = sizeof(SFPacketHeader); }
	void ReadEnd(){}

	inline SFPacket& SFPacket::operator << (BYTE Data)
	{
		if (GetPacketSize() + sizeof(BYTE) > m_packetMaxSize)
			return *this;

		memcpy(&m_pPacketBuffer[GetPacketSize()], &Data, sizeof(BYTE));
		m_pHeader->dataSize += sizeof(BYTE);
		
		return *this;
	}

	inline SFPacket& SFPacket::operator << (CHAR Data)
	{
		if (GetPacketSize() + sizeof(CHAR) > m_packetMaxSize)
			return *this;

		memcpy(&m_pPacketBuffer[GetPacketSize()], &Data, sizeof(CHAR));
		m_pHeader->dataSize += sizeof(CHAR);

		return *this;
	}

	inline SFPacket& SFPacket::operator << (SHORT Data)
	{
		if (GetPacketSize() + sizeof(SHORT) > m_packetMaxSize)
			return *this;

		memcpy(&m_pPacketBuffer[GetPacketSize()], &Data, sizeof(SHORT));
		m_pHeader->dataSize += sizeof(SHORT);

		return *this;
	}

	inline SFPacket& SFPacket::operator << (USHORT Data)
	{
		if (GetPacketSize() + sizeof(USHORT) > m_packetMaxSize)
			return *this;

		memcpy(&m_pPacketBuffer[GetPacketSize()], &Data, sizeof(USHORT));
		m_pHeader->dataSize += sizeof(USHORT);
		
		return *this;
	}

	inline SFPacket& SFPacket::operator << (int Data)
	{
		if (GetPacketSize() + sizeof(int) > m_packetMaxSize)
			return *this;

		memcpy(&m_pPacketBuffer[GetPacketSize()], &Data, sizeof(int));
		m_pHeader->dataSize += sizeof(int);

		return *this;
	}

	inline SFPacket& SFPacket::operator << (DWORD Data)
	{
		if (GetPacketSize() + sizeof(DWORD) > m_packetMaxSize)
			return *this;

		memcpy(&m_pPacketBuffer[GetPacketSize()], &Data, sizeof(DWORD));
		m_pHeader->dataSize += sizeof(DWORD);

		return *this;
	}

	inline SFPacket& SFPacket::operator << (float Data)
	{
		if (GetPacketSize() + sizeof(float) > m_packetMaxSize)
			return *this;

		memcpy(&m_pPacketBuffer[GetPacketSize()], &Data, sizeof(float));
		m_pHeader->dataSize += sizeof(float);
		
		return *this;
	}

	inline SFPacket& SFPacket::operator << (char* szStr)
	{
		int len = (int)strlen(szStr);

		if (len <= 0 || GetPacketSize() + len + 1 > m_packetMaxSize)
			return *this;

		memcpy(&m_pPacketBuffer[GetPacketSize()], szStr, len);
		m_pHeader->dataSize += (USHORT)len;
		m_pPacketBuffer[GetPacketSize()] = 0;
		m_pHeader->dataSize++;
		
		return *this;
	}

	void SFPacket::Write(const BYTE* pBuffer, int bufferSize)
	{
		if (GetPacketSize() + bufferSize > m_packetMaxSize)
			return;

		memcpy(&m_pPacketBuffer[GetPacketSize()], pBuffer, bufferSize);
		m_pHeader->dataSize += (USHORT)bufferSize;

	}

	inline SFPacket& SFPacket::operator >> (BYTE& Data)
	{
		if (m_usCurrentReadPosition + sizeof(BYTE) > m_packetMaxSize)
			return *this;

		memcpy(&Data, &m_pPacketBuffer[m_usCurrentReadPosition], sizeof(BYTE));
		m_usCurrentReadPosition += sizeof(BYTE);
		
		return *this;
	}

	inline SFPacket& SFPacket::operator >> (CHAR& Data)
	{
		if (m_usCurrentReadPosition + sizeof(CHAR) > m_packetMaxSize)
			return *this;

		memcpy(&Data, &m_pPacketBuffer[m_usCurrentReadPosition], sizeof(CHAR));
		m_usCurrentReadPosition += sizeof(CHAR);

		return *this;
	}

	inline SFPacket& SFPacket::operator >> (SHORT& Data)
	{
		if (m_usCurrentReadPosition + sizeof(SHORT) > m_packetMaxSize)
			return *this;

		memcpy(&Data, &m_pPacketBuffer[m_usCurrentReadPosition], sizeof(SHORT));
		m_usCurrentReadPosition += sizeof(SHORT);

		return *this;
	}

	inline SFPacket& SFPacket::operator >> (USHORT& Data)
	{
		if (m_usCurrentReadPosition + sizeof(USHORT) > m_packetMaxSize)
			return *this;

		memcpy(&Data, &m_pPacketBuffer[m_usCurrentReadPosition], sizeof(USHORT));
		m_usCurrentReadPosition += sizeof(USHORT);

		return *this;
	}

	inline SFPacket& SFPacket::operator >> (int& Data)
	{
		if (m_usCurrentReadPosition + sizeof(int) > m_packetMaxSize)
			return *this;

		memcpy(&Data, &m_pPacketBuffer[m_usCurrentReadPosition], sizeof(int));
		m_usCurrentReadPosition += sizeof(int);

		return *this;
	}

	inline SFPacket& SFPacket::operator >> (DWORD& Data)
	{
		if (m_usCurrentReadPosition + sizeof(DWORD) > m_packetMaxSize)
			return *this;

		memcpy(&Data, &m_pPacketBuffer[m_usCurrentReadPosition], sizeof(DWORD));
		m_usCurrentReadPosition += sizeof(DWORD);

		return *this;
	}

	inline SFPacket& SFPacket::operator >> (float& Data)
	{
		if (m_usCurrentReadPosition + sizeof(float) > m_packetMaxSize)
			return *this;

		memcpy(&Data, &m_pPacketBuffer[m_usCurrentReadPosition], sizeof(float));
		m_usCurrentReadPosition += sizeof(float);

		return *this;
	}

	inline SFPacket& SFPacket::operator >> (char* szStr)
	{
		int strLen = (int)strlen((char*)&m_pPacketBuffer[m_usCurrentReadPosition]);
		if (m_usCurrentReadPosition + strLen + 1 > m_packetMaxSize)
			return *this;

		strcpy_s(szStr, strLen + 1, (char*)&m_pPacketBuffer[m_usCurrentReadPosition]);
		m_usCurrentReadPosition += (USHORT)(strLen + 1);

		return *this;
	}

	void SFPacket::Read(BYTE* pBuffer, int bufferSize)
	{
		if (m_usCurrentReadPosition + bufferSize > m_packetMaxSize)
			return;

		memcpy(pBuffer, &m_pPacketBuffer[m_usCurrentReadPosition], bufferSize);
		m_usCurrentReadPosition += (USHORT)bufferSize;
	}

	virtual BasePacket* Clone() override;

protected:

private:
	SFPacketHeader* m_pHeader;
	BYTE* m_pPacketBuffer;
	USHORT m_usCurrentReadPosition;

	static SFFastCRC m_FastCRC;
	bool m_bEncoded;

	static USHORT m_packetMaxSize;	
};