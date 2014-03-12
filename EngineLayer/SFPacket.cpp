#include "StdAfx.h"
#include "SFPacket.h"
#include "SFCompressor.h"
#include "SFCompressLzf.h"
#include "SFCompressZLib.h"
#include "SFChecksum.h"

SFFastCRC SFPacket::m_FastCRC;

SFPacket::SFPacket(USHORT packetID)
{
	m_Header.packetID = packetID;

	Initialize();
}

SFPacket::SFPacket()
{
	m_Header.packetID = 0;

	Initialize();
}

SFPacket::~SFPacket(void)
{
}

BOOL SFPacket::Initialize()
{
	m_Header.packetOption = 0;
	m_Header.dataCRC = 0;
	m_Header.dataSize = 0;

	ResetDataBuffer();

	m_usCurrentReadPosition = 0;

	m_bEncoded = false;

	return TRUE;
}

void SFPacket::ResetDataBuffer()
{
	memset(m_pPacketData, 0, sizeof(MAX_PACKET_DATA));
}

bool SFPacket::Encode()
{
	int PacketOption = CGSF_PACKET_OPTION;

	if (m_bEncoded == true)
		return true;

	if(GetDataSize() < 0 || GetDataSize() > MAX_PACKET_DATA)
	{
		SFASSERT(0);
		return false;
	}

	if(GetDataSize() == 0)
	{
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

		m_Header.dataCRC = dwDataCRC;
	}

	m_Header.SetPacketOption(PacketOption);

	m_bEncoded = true;

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
		if(FALSE == SFEncrytion<SFEncryptionXOR>::Decrypt((BYTE*)GetDataBuffer(), GetDataSize()))
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

		SetDataSize(DestSize);
	}

	BasePacket::SetPacketID(pHeader->packetID);

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

	if(TRUE != Result || dwDataCRC != GetHeader()->dataCRC)
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