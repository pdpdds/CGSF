#pragma once
#include <sqltypes.h>
#include "BasePacket.h"
#include "Macro.h"
#include <stdio.h>

#define MAX_MESSAGE_BUFFER 512

class SFMessage : public BasePacket
{
public:
	SFMessage() {Reset();}
	virtual ~SFMessage(){}

	void Reset()
	{
		m_usCommand = 0;

		ResetData();
	}

	void ResetData()
	{
		memset(m_aDataBuffer, 0, sizeof(BYTE) * MAX_MESSAGE_BUFFER);
		m_usDataSize = 0;
		m_usCurrentReadPosition = 0;
	}

	USHORT GetCommand() { return m_usCommand;}
	USHORT GetDataSize() { return m_usDataSize;}
	BYTE* GetData() { return m_aDataBuffer;}
	
	void Initialize(USHORT usCommand) { m_usCommand = usCommand; ResetData();}

	void ReadStart(){m_usCurrentReadPosition = 0;}
	void ReadEnd(){}

	inline SFMessage& SFMessage::operator << (BYTE Data)
	{
		memcpy(&m_aDataBuffer[m_usDataSize], &Data, sizeof(BYTE));
		m_usDataSize += sizeof(BYTE);

		SFASSERT(m_usDataSize <= MAX_MESSAGE_BUFFER);
		return *this;
	}

	inline SFMessage& SFMessage::operator << (CHAR Data)
	{
		memcpy(&m_aDataBuffer[m_usDataSize], &Data, sizeof(CHAR));
		m_usDataSize += sizeof(CHAR);

		SFASSERT(m_usDataSize <= MAX_MESSAGE_BUFFER);
		return *this;
	}

	inline SFMessage& SFMessage::operator << (SHORT Data)
	{
		memcpy(&m_aDataBuffer[m_usDataSize], &Data, sizeof(SHORT));
		m_usDataSize += sizeof(SHORT);

		SFASSERT(m_usDataSize <= MAX_MESSAGE_BUFFER);
		return *this;
	}

	inline SFMessage& SFMessage::operator << (USHORT Data)
	{
		memcpy(&m_aDataBuffer[m_usDataSize], &Data, sizeof(USHORT));
		m_usDataSize += sizeof(USHORT);

		SFASSERT(m_usDataSize <= MAX_MESSAGE_BUFFER);
		return *this;
	}

	inline SFMessage& SFMessage::operator << (int Data)
	{
		memcpy(&m_aDataBuffer[m_usDataSize], &Data, sizeof(int));
		m_usDataSize += sizeof(int);

		SFASSERT(m_usDataSize <= MAX_MESSAGE_BUFFER);
		return *this;
	}

	inline SFMessage& SFMessage::operator << (DWORD Data)
	{
		memcpy(&m_aDataBuffer[m_usDataSize], &Data, sizeof(DWORD));
		m_usDataSize += sizeof(DWORD);

		SFASSERT(m_usDataSize <= MAX_MESSAGE_BUFFER);
		return *this;
	}

	inline SFMessage& SFMessage::operator << (float Data)
	{
		memcpy(&m_aDataBuffer[m_usDataSize], &Data, sizeof(float));
		m_usDataSize += sizeof(float);

		SFASSERT(m_usDataSize <= MAX_MESSAGE_BUFFER);
		return *this;
	}

	inline SFMessage& SFMessage::operator << (char* szStr)
	{
		int len = (int)strlen(szStr);
		SFASSERT(len > 0);

		memcpy(&m_aDataBuffer[m_usDataSize], szStr, len);
		m_usDataSize += (USHORT)len;
		m_aDataBuffer[m_usDataSize] = 0;
		m_usDataSize++;

		SFASSERT(m_usDataSize <= MAX_MESSAGE_BUFFER);
		return *this;
	}

	inline SFMessage& SFMessage::operator << (TIMESTAMP_STRUCT& Data)
	{
		memcpy(&m_aDataBuffer[m_usDataSize], &Data, sizeof(TIMESTAMP_STRUCT));
		m_usDataSize += sizeof(TIMESTAMP_STRUCT);

		SFASSERT(m_usDataSize <= MAX_MESSAGE_BUFFER);
		return *this;
	}

	inline SFMessage& SFMessage::operator >> (BYTE& Data)
	{
		memcpy(&Data, &m_aDataBuffer[m_usCurrentReadPosition], sizeof(BYTE));
		m_usCurrentReadPosition += sizeof(BYTE);

		SFASSERT(m_usCurrentReadPosition <= MAX_MESSAGE_BUFFER);
		return *this;
	}

	inline SFMessage& SFMessage::operator >> (CHAR& Data)
	{
		memcpy(&Data, &m_aDataBuffer[m_usCurrentReadPosition], sizeof(CHAR));
		m_usCurrentReadPosition += sizeof(CHAR);

		SFASSERT(m_usCurrentReadPosition <= MAX_MESSAGE_BUFFER);
		return *this;
	}

	inline SFMessage& SFMessage::operator >> (SHORT& Data)
	{
		memcpy(&Data, &m_aDataBuffer[m_usCurrentReadPosition], sizeof(SHORT));
		m_usCurrentReadPosition += sizeof(SHORT);

		SFASSERT(m_usCurrentReadPosition <= MAX_MESSAGE_BUFFER);
		return *this;
	}

	inline SFMessage& SFMessage::operator >> (USHORT& Data)
	{
		memcpy(&Data, &m_aDataBuffer[m_usCurrentReadPosition], sizeof(USHORT));
		m_usCurrentReadPosition += sizeof(USHORT);

		SFASSERT(m_usCurrentReadPosition <= MAX_MESSAGE_BUFFER);
		return *this;
	}

	inline SFMessage& SFMessage::operator >> (int& Data)
	{
		memcpy(&Data, &m_aDataBuffer[m_usCurrentReadPosition], sizeof(int));
		m_usCurrentReadPosition += sizeof(int);

		SFASSERT(m_usCurrentReadPosition <= MAX_MESSAGE_BUFFER);
		return *this;
	}

	inline SFMessage& SFMessage::operator >> (DWORD& Data)
	{
		memcpy(&Data, &m_aDataBuffer[m_usCurrentReadPosition], sizeof(DWORD));
		m_usCurrentReadPosition += sizeof(DWORD);

		SFASSERT(m_usCurrentReadPosition <= MAX_MESSAGE_BUFFER);
		return *this;
	}

	inline SFMessage& SFMessage::operator >> (float& Data)
	{
		memcpy(&Data, &m_aDataBuffer[m_usCurrentReadPosition], sizeof(float));
		m_usCurrentReadPosition += sizeof(float);

		SFASSERT(m_usCurrentReadPosition <= MAX_MESSAGE_BUFFER);
		return *this;
	}

	inline SFMessage& SFMessage::operator >> (TIMESTAMP_STRUCT& Data)
	{
		memcpy(&Data, &m_aDataBuffer[m_usCurrentReadPosition], sizeof(TIMESTAMP_STRUCT));
		m_usCurrentReadPosition += sizeof(TIMESTAMP_STRUCT);

		SFASSERT(m_usCurrentReadPosition <= MAX_MESSAGE_BUFFER);
		return *this;
	}

	inline SFMessage& SFMessage::operator >> (char* szStr)
	{
		strcpy_s(szStr, strlen((char*)&m_aDataBuffer[m_usCurrentReadPosition])+1, (char*)&m_aDataBuffer[m_usCurrentReadPosition]);
		m_usCurrentReadPosition += (USHORT)(strlen((char*)&m_aDataBuffer[m_usCurrentReadPosition]) + 1);

		SFASSERT(m_usCurrentReadPosition <= MAX_MESSAGE_BUFFER);

		return *this;
	}

	void SFMessage::Read(BYTE* pBuffer, int BufferSize)
	{
		memcpy(pBuffer, &m_aDataBuffer[m_usCurrentReadPosition], BufferSize);
		m_usCurrentReadPosition += (USHORT)BufferSize;

		SFASSERT(m_usCurrentReadPosition <= MAX_MESSAGE_BUFFER);
	}

	void SFMessage::Write(const BYTE* pBuffer, int BufferSize)
	{
		memcpy(&m_aDataBuffer[m_usDataSize], pBuffer, BufferSize);
		m_usDataSize += (USHORT)BufferSize;

		SFASSERT(m_usDataSize <= MAX_MESSAGE_BUFFER);
	}

protected:

private:
	USHORT m_usCommand;
	BYTE m_aDataBuffer[MAX_MESSAGE_BUFFER];
	USHORT m_usDataSize;
	USHORT m_usCurrentReadPosition;
};