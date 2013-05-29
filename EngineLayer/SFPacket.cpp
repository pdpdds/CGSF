#include "StdAfx.h"
#include "SFPacket.h"
#include "SFCompressor.h"
#include "SFCompressLzf.h"
#include "SFCompressZLib.h"
#include "SFChecksum.h"

SFFastCRC SFPacket::m_FastCRC;

SFPacket::SFPacket(void)
{
	Initialize();
}

SFPacket::~SFPacket(void)
{
}

BOOL SFPacket::Initialize()
{
	memset(&m_Header, 0, sizeof(SFPacketHeader));

	ResetDataBuffer();

	return TRUE;
}

void SFPacket::ResetDataBuffer()
{
	memset(m_pPacketData, 0, sizeof(MAX_PACKET_DATA));
}

bool SFPacket::Encode()
{
	int PacketOption = CGSF_PACKET_OPTION;

	if(GetDataSize() < 0 || GetDataSize() > MAX_PACKET_DATA)
	{
		SFASSERT(0);
		return false;
	}

	if(GetDataSize() == 0)
	{
		m_Header.PacketLen = sizeof(SFPacketHeader);
		return true;
	}

	BYTE pDestBuf[4096] = {0,};
	int DestSize = 4096;

	DWORD dwResult = 0;

	if (PacketOption & PACKET_OPTION_COMPRESS && GetDataSize() >= PACKET_COMPRESS_LIMIT)
	{
		dwResult = SFCompressor<SFCompressLzf>::GetCompressor()->Compress(pDestBuf, DestSize, GetDataBuffer(), GetDataSize());

		 if(dwResult != TRUE)
		 {
			 SFASSERT(0);
			 return false;
		 }

		 m_Header.PacketLen = sizeof(SFPacketHeader) + DestSize;
		 memcpy(m_pPacketData, pDestBuf, DestSize);
		 SetDataSize(DestSize);
	}
	else
	{
		PacketOption = PacketOption & (~PACKET_OPTION_COMPRESS);
	}

	if (PacketOption & PACKET_OPTION_ENCRYPTION)
	{
		if(false == SFEncrytion<SFEncryptionXOR>::Encrypt((BYTE*)m_pPacketData, GetDataSize()))
		{
			SFASSERT(0);
			return false;
		}
	}

	DWORD dwDataCRC = 0;

	if (PacketOption & PACKET_OPTION_DATACRC)
	{
		BOOL Result = SFPacket::GetDataCRC((BYTE*)m_pPacketData, GetDataSize(), dwDataCRC);
		
		if(false == Result)
		{
			SFASSERT(0);
			return false;
		}

		m_Header.DataCRC = dwDataCRC;
	}

	m_Header.SetPacketOption(PacketOption);

	return true;
}

BOOL SFPacket::Decode(int& ErrorCode)
{
	SFPacketHeader* pHeader = GetHeader();

	if(TRUE == pHeader->CheckDataCRC())
	{
		BOOL Result = CheckDataCRC();

		if(TRUE != Result)
		{
			//SFLOG_WARN(L"Packet CRC Check Fail!! %d %d", pHeader->DataCRC, dwDataCRC);
			ErrorCode = PACKETIO_ERROR_DATA_CRC;
			return FALSE;
		}
	}

	if (TRUE == pHeader->CheckEncryption())
	{	
		if(FALSE == SFEncrytion<SFEncryptionXOR>::Decrypt((BYTE*)GetDataBuffer(), pHeader->PacketLen - sizeof(SFPacketHeader)))
		{
			SFASSERT(0);
			ErrorCode = PACKETIO_ERROR_DATA_ENCRYPTION;
			return FALSE;
		}
	}

	if(TRUE == pHeader->CheckCompressed())
	{
		BYTE pSrcBuf[MAX_PACKET_DATA] = {0,};
		int DestSize = MAX_PACKET_DATA;

		memcpy(pSrcBuf, GetDataBuffer(), GetDataSize());
		ResetDataBuffer();

		if(FALSE == SFCompressor<SFCompressLzf>::GetCompressor()->Uncompress(GetDataBuffer(), DestSize, pSrcBuf, GetDataSize()))
		{
			//SFLOG_WARN(L"Packet Uncompress Fail!! %d %d", pHeader->DataCRC, dwDataCRC);

			ErrorCode = PACKETIO_ERROR_DATA_COMPRESS;

			return FALSE;
		}

		pHeader->PacketLen = DestSize + sizeof(SFPacketHeader);
	}

	return TRUE;
}

BOOL SFPacket::GetDataCRC(BYTE* pDataBuf, DWORD DataSize, DWORD& dwDataCRC)
{
	BOOL Result = m_FastCRC.GetZLibCRC((BYTE*)pDataBuf, DataSize, dwDataCRC);

	if(TRUE != Result)
	{
		SFASSERT(0);
		return FALSE;
	}

	return TRUE;
}

BOOL SFPacket::CheckDataCRC()
{
	 DWORD dwDataCRC = 0;

	BOOL Result = m_FastCRC.GetZLibCRC((BYTE*)m_pPacketData, GetDataSize(), dwDataCRC);

	if(TRUE != Result || dwDataCRC != GetHeader()->DataCRC)
	{
		SFASSERT(0);
		return FALSE;
	}

	//DWORD dwResult = SFChecksum::FromBuffer((BYTE*)m_pPacketData, SrcSize, dwDataCRC);


	/*if(ERROR_SUCCESS != dwResult)
	{
	SFASSERT(0);
	return FALSE;
	}*/

	return TRUE;
}