#pragma once
#include "SFChecksum.h"
#include "SFPacketIOBuffer.h"
#include "SFEncryption.h"
#include "SFEncryptionXOR.h"

class SFPacket;
class SFPacketIOBuffer;

template <typename T>
class SFPacketAnalyzer
{
public:
	SFPacketAnalyzer();
	virtual ~SFPacketAnalyzer(void);

	BOOL Reset();
	BOOL AddTransferredData(char* pBuffer, DWORD dwTransferred);
	BOOL GetPacket(SFPacket& Packet, int& ErrorCode);

	static T& GetCompressor(){return m_Compress;}

protected:
	BOOL Initialize();

private:
	SFPacketIOBuffer* m_pPacketIOBuffer;
	static T m_Compress;
};


template <typename T>
T SFPacketAnalyzer<T>::m_Compress;

template <typename T>
SFPacketAnalyzer<T>::SFPacketAnalyzer()
{
	Initialize();
}

template <typename T>
SFPacketAnalyzer<T>::~SFPacketAnalyzer(void)
{
	if(m_pPacketIOBuffer)
		delete m_pPacketIOBuffer;

	m_pPacketIOBuffer = NULL;
}

template <typename T>
BOOL SFPacketAnalyzer<T>::Initialize()
{
	m_pPacketIOBuffer = new SFPacketIOBuffer();
	m_pPacketIOBuffer->AllocIOBuf(PACKETIO_SIZE);

	return TRUE;
}

template <typename T>
BOOL SFPacketAnalyzer<T>::Reset()
{
	m_pPacketIOBuffer->InitIOBuf();

	return TRUE;
}

template <typename T>
BOOL SFPacketAnalyzer<T>::AddTransferredData(char* pBuffer, DWORD dwTransferred)
{
	m_pPacketIOBuffer->AppendData(pBuffer, dwTransferred);

	return TRUE;
}

template <typename T>
BOOL SFPacketAnalyzer<T>::GetPacket(SFPacket& Packet, int& ErrorCode)
{
	if(FALSE == m_pPacketIOBuffer->GetPacket(&Packet, ErrorCode))
	{
		return FALSE;
	}

	SFPacketHeader* pHeader = Packet.GetHeader();

	if(TRUE == pHeader->CheckDataCRC())
	{
		DWORD dwDataCRC = 0;
		
		BOOL Result = SFPacket::GetDataCRC((BYTE*)Packet.GetDataBuffer(), pHeader->PacketLen - sizeof(SFPacketHeader), dwDataCRC);

		if(TRUE != Result || pHeader->DataCRC != dwDataCRC)
		{
			//SFLOG_WARN(L"Packet CRC Check Fail!! %d %d", pHeader->DataCRC, dwDataCRC);

			ErrorCode = PACKETIO_ERROR_DATA_CRC;

			return FALSE;
		}
	}

	if (TRUE == pHeader->CheckEncryption())
	{	
		if(FALSE == SFEncrytion<SFEncryptionXOR>::Decrypt((BYTE*)Packet.GetDataBuffer(), pHeader->PacketLen - sizeof(SFPacketHeader)))
		{
			SFASSERT(0);
			return FALSE;
		}
	}

	if(TRUE == pHeader->CheckCompressed())
	{
		BYTE pSrcBuf[MAX_PACKET_DATA] = {0,};
		int DestSize = MAX_PACKET_DATA;

		memcpy(pSrcBuf, Packet.GetDataBuffer(), Packet.GetDataSize());
		Packet.ResetDataBuffer();

		if(FALSE == m_Compress.Uncompress(Packet.GetDataBuffer(), DestSize, pSrcBuf, Packet.GetDataSize()))
		{
			//SFLOG_WARN(L"Packet Uncompress Fail!! %d %d", pHeader->DataCRC, dwDataCRC);

			ErrorCode = PACKETIO_ERROR_DATA_COMPRESS;

			return FALSE;
		}

		pHeader->PacketLen = DestSize + sizeof(SFPacketHeader);
	}

	return TRUE;
}