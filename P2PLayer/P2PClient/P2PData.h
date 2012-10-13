#pragma once

#define MAX_P2P_DATA 2048

class P2PData
{
public:
	P2PData() {Reset();}
	virtual ~P2PData(){}

	void Reset()
	{
	
		ResetData();
	}

	void ResetData()
	{
		memset(m_aDataBuffer, 0, sizeof(BYTE) * MAX_P2P_DATA);
		m_usDataSize = 0;
		m_usCurrentReadPosition = 0;
	}

	USHORT GetDataSize() { return m_usDataSize;}
	BYTE* GetData() { return m_aDataBuffer;}
	
	void Initialize() { ResetData();}

	void ReadStart(){m_usCurrentReadPosition = 0;}
	void ReadEnd(){}

	void P2PData::Read(BYTE* pBuffer, int BufferSize)
	{
		memcpy(pBuffer, &m_aDataBuffer[m_usCurrentReadPosition], BufferSize);
		m_usCurrentReadPosition += BufferSize;

		assert(m_usCurrentReadPosition <= MAX_P2P_DATA);
	}

	void P2PData::Write(const BYTE* pBuffer, int BufferSize)
	{
		memcpy(&m_aDataBuffer[m_usDataSize], pBuffer, BufferSize);
		m_usDataSize += BufferSize;

		assert(m_usDataSize <= MAX_P2P_DATA);
	}

protected:

private:
	BYTE m_aDataBuffer[MAX_P2P_DATA];
	USHORT m_usDataSize;
	USHORT m_usCurrentReadPosition;
};