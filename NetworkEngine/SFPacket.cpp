#include "StdAfx.h"
#include "SFPacket.h"
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

BOOL SFPacket::MakePacket(BYTE* pSrcBuf, int SrcSize, int PacketOption)
{
	if(SrcSize < 0 || SrcSize > MAX_PACKET_DATA)
	{
		SFASSERT(0);
		return FALSE;
	}

	if(SrcSize == 0)
	{
		m_Header.PacketLen = sizeof(SFPacketHeader);
		return TRUE;
	}

	BYTE pDestBuf[4096] = {0,};
	int DestSize = 4096;

	DWORD dwResult = 0;

	if (PacketOption & PACKET_OPTION_COMPRESS && SrcSize >= PACKET_COMPRESS_LIMIT)
	{
		dwResult = SFPacketAnalyzer<SFCompressLzf>::GetCompressor().Compress(pDestBuf, DestSize, pSrcBuf, SrcSize);

		 if(dwResult != TRUE)
		 {
			 SFASSERT(0);
			 return FALSE;
		 }

		 m_Header.PacketLen = sizeof(SFPacketHeader) + DestSize;
		 memcpy(m_pPacketData, pDestBuf, DestSize);
		 SrcSize = DestSize;
	}
	else
	{
		PacketOption = PacketOption & (~PACKET_OPTION_COMPRESS);
		m_Header.PacketLen = sizeof(SFPacketHeader) + SrcSize;
		memcpy(m_pPacketData, pSrcBuf, SrcSize);
	}

	if (PacketOption & PACKET_OPTION_ENCRYPTION)
	{
		if(FALSE == SFEncrytion<SFEncryptionXOR>::Encrypt((BYTE*)m_pPacketData, SrcSize))
		{
			SFASSERT(0);
			return FALSE;
		}
	}

	DWORD dwDataCRC = 0;

	if (PacketOption & PACKET_OPTION_DATACRC)
	{
		BOOL Result = SFPacket::GetDataCRC((BYTE*)m_pPacketData, SrcSize, dwDataCRC);
		
		if(FALSE == Result)
		{
			SFASSERT(0);
			return FALSE;
		}

		m_Header.DataCRC = dwDataCRC;
	}

	m_Header.SetPacketOption(PacketOption);

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

	//DWORD dwResult = SFChecksum::FromBuffer((BYTE*)m_pPacketData, SrcSize, dwDataCRC);


	/*if(ERROR_SUCCESS != dwResult)
	{
	SFASSERT(0);
	return FALSE;
	}*/

	return TRUE;

}