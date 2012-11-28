#include "stdafx.h"
#include "SFDefaultPacketProtocol.h"
#include "SFCompressor.h"

SFDefaultPacketProtocol::SFDefaultPacketProtocol(void)
{
	Initialize();
}

SFDefaultPacketProtocol::~SFDefaultPacketProtocol(void)
{
	if(m_pPacketIOBuffer)
		delete m_pPacketIOBuffer;

	m_pPacketIOBuffer = NULL;
}

BOOL SFDefaultPacketProtocol::Initialize()
{
	m_pPacketIOBuffer = new SFPacketIOBuffer();
	m_pPacketIOBuffer->AllocIOBuf(PACKETIO_SIZE);

	return TRUE;
}

BasePacket* SFDefaultPacketProtocol::GetPacket(int& ErrorCode)
{
	SFPacket* pPacket = PacketPoolSingleton::instance()->Alloc();
	pPacket->Initialize();

	if(FALSE == m_pPacketIOBuffer->GetPacket(pPacket, ErrorCode))
	{
		PacketPoolSingleton::instance()->Release(pPacket);
		return NULL;
	}

	SFPacketHeader* pHeader = pPacket->GetHeader();

	if(TRUE == pHeader->CheckDataCRC())
	{
		DWORD dwDataCRC = 0;
		
		BOOL Result = SFPacket::GetDataCRC((BYTE*)pPacket->GetDataBuffer(), pHeader->PacketLen - sizeof(SFPacketHeader), dwDataCRC);

		if(TRUE != Result || pHeader->DataCRC != dwDataCRC)
		{
			//SFLOG_WARN(L"Packet CRC Check Fail!! %d %d", pHeader->DataCRC, dwDataCRC);

			ErrorCode = PACKETIO_ERROR_DATA_CRC;
			PacketPoolSingleton::instance()->Release(pPacket);

			return NULL;
		}
	}

	if (TRUE == pHeader->CheckEncryption())
	{	
		if(FALSE == SFEncrytion<SFEncryptionXOR>::Decrypt((BYTE*)pPacket->GetDataBuffer(), pHeader->PacketLen - sizeof(SFPacketHeader)))
		{
			SFASSERT(0);
			PacketPoolSingleton::instance()->Release(pPacket);
			ErrorCode = PACKETIO_ERROR_DATA_ENCRYPTION;
			return FALSE;
		}
	}

	if(TRUE == pHeader->CheckCompressed())
	{
		BYTE pSrcBuf[MAX_PACKET_DATA] = {0,};
		int DestSize = MAX_PACKET_DATA;

		memcpy(pSrcBuf, pPacket->GetDataBuffer(), pPacket->GetDataSize());
		pPacket->ResetDataBuffer();

		if(FALSE == SFCompressor<SFCompressLzf>::GetCompressor()->Uncompress(pPacket->GetDataBuffer(), DestSize, pSrcBuf, pPacket->GetDataSize()))
		{
			//SFLOG_WARN(L"Packet Uncompress Fail!! %d %d", pHeader->DataCRC, dwDataCRC);

			PacketPoolSingleton::instance()->Release(pPacket);
			ErrorCode = PACKETIO_ERROR_DATA_COMPRESS;

			return FALSE;
		}

		pHeader->PacketLen = DestSize + sizeof(SFPacketHeader);
	}

	return pPacket;
}

BOOL SFDefaultPacketProtocol::AddTransferredData(char* pBuffer, DWORD dwTransferred)
{
	m_pPacketIOBuffer->AppendData(pBuffer, dwTransferred);

	return TRUE;
}

BOOL SFDefaultPacketProtocol::Reset()
{
	m_pPacketIOBuffer->InitIOBuf();

	return TRUE;
}