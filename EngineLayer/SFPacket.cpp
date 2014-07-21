#include "StdAfx.h"
#include "SFPacket.h"
#include "SFCompressor.h"
#include "SFCompressLzf.h"
#include "SFCompressZLib.h"
#include "SFChecksum.h"
#include "SFCGSFPacketProtocol.h"

USHORT SFPacket::m_packetMaxSize = MAX_PACKET_SIZE;

SFFastCRC SFPacket::m_FastCRC;

SFPacket::SFPacket(USHORT packetID)
{	
	m_pPacketBuffer = new BYTE[m_packetMaxSize];
	m_pHeader = (SFPacketHeader*)m_pPacketBuffer;
	
	Initialize();

	m_pHeader->packetID = packetID;
}

SFPacket::SFPacket()
{	
	m_pPacketBuffer = new BYTE[m_packetMaxSize];
	m_pHeader = (SFPacketHeader*)m_pPacketBuffer;
	m_pHeader->packetID = 0;

	Initialize();
}

SFPacket::~SFPacket(void)
{
	SF_SAFE_RELEASE(m_pPacketBuffer);
}

BOOL SFPacket::Initialize()
{
	m_pHeader->packetOption = 0;
	m_pHeader->dataCRC = 0;
	m_pHeader->dataSize = 0;

	ResetBuffer();

	m_usCurrentReadPosition = sizeof(SFPacketHeader);

	m_bEncoded = false;

	return TRUE;
}

void SFPacket::ResetBuffer()
{
	memset(m_pPacketBuffer, 0, m_packetMaxSize);
}

void SFPacket::ResetDataBuffer()
{
	memset(m_pPacketBuffer + sizeof(SFPacketHeader), 0, m_packetMaxSize - sizeof(SFPacketHeader));
}

bool SFPacket::Encode(unsigned short packetSize, int packetOption)
{

	if (m_bEncoded == true)
		return true;

	if (GetDataSize() < 0 || GetDataSize() > m_packetMaxSize - sizeof(SFPacketHeader))
	{
		SFASSERT(0);
		return false;
	}

	if(GetDataSize() == 0)
	{
		return true;
	}

	BYTE pDestBuf[MAX_PACKET_SIZE] = { 0, };
	int destSize = packetSize - sizeof(SFPacketHeader);

	DWORD dwResult = 0;

	if (packetOption & PACKET_OPTION_COMPRESS && GetDataSize() >= PACKET_COMPRESS_LIMIT)
	{
		dwResult = SFCompressor<SFCompressLzf>::GetCompressor()->Compress(pDestBuf, destSize, GetData(), GetDataSize());

		 if(dwResult != TRUE)
		 {
			 SFASSERT(0);
			 return false;
		 }

		 memcpy(GetData(), pDestBuf, destSize);
		 SetDataSize((USHORT)destSize);
	}
	else
	{
		packetOption = packetOption & (~PACKET_OPTION_COMPRESS);
	}

	if (packetOption & PACKET_OPTION_ENCRYPTION)
	{
		if(false == SFEncrytion<SFEncryptionXOR>::Encrypt((BYTE*)GetData(), GetDataSize()))
		{
			SFASSERT(0);
			return false;
		}
	}

	DWORD dwDataCRC = 0;

	if (packetOption & PACKET_OPTION_DATACRC)
	{
		BOOL Result = SFPacket::GetDataCRC((BYTE*)GetData(), GetDataSize(), dwDataCRC);
		
		if(false == Result)
		{
			SFASSERT(0);
			return false;
		}

		m_pHeader->dataCRC = dwDataCRC;
	}

	m_pHeader->SetPacketOption(packetOption);

	m_bEncoded = true;

	return true;
}

bool SFPacket::Decode(unsigned short packetSize, int& errorCode)
{
	SFPacketHeader* pHeader = GetHeader();

	if(TRUE == pHeader->CheckDataCRC())
	{
		BOOL result = CheckDataCRC();

		if (TRUE != result)
		{
			LOG(WARNING) << "Packet CRC Check Fail!!";
			
			errorCode = PACKETIO_ERROR_DATA_CRC;
			return FALSE;
		}
	}

	if (TRUE == pHeader->CheckEncryption())
	{	
		if(FALSE == SFEncrytion<SFEncryptionXOR>::Decrypt((BYTE*)GetData(), GetDataSize()))
		{
			SFASSERT(0);
			errorCode = PACKETIO_ERROR_DATA_ENCRYPTION;
			return FALSE;
		}
	}

	if(TRUE == pHeader->CheckCompressed())
	{
		BYTE pSrcBuf[MAX_IO_SIZE] = { 0, };
		int destSize = packetSize;

		memcpy(pSrcBuf, GetData(), GetDataSize());
		ResetDataBuffer();

		if (FALSE == SFCompressor<SFCompressLzf>::GetCompressor()->Uncompress(GetData(), destSize, pSrcBuf, GetDataSize()))
		{
			//SFLOG_WARN(L"Packet Uncompress Fail!! %d %d", pHeader->DataCRC, dwDataCRC);

			errorCode = PACKETIO_ERROR_DATA_COMPRESS;

			return FALSE;
		}

		if (destSize + sizeof(SFPacketHeader) > packetSize)
		{
			errorCode = PACKETIO_ERROR_DATA_COMPRESS;
			return FALSE;
		}

		SetDataSize((USHORT)destSize);
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

	BOOL result = GetDataCRC((BYTE*)GetData(), GetDataSize(), dwDataCRC);

	if (TRUE != result || dwDataCRC != GetHeader()->dataCRC)
	{
		SFASSERT(0);
		return FALSE;
	}

	//DWORD dwResult = SFChecksum::FromBuffer((BYTE*)GetDataBuffer(), SrcSize, dwDataCRC);


	/*if(ERROR_SUCCESS != dwResult)
	{
	SFASSERT(0);
	return FALSE;
	}*/

	return TRUE;
}

BasePacket* SFPacket::Clone()
{
	SFPacket* pClone = new SFPacket();
	pClone->CopyBaseHeader(this);
	memcpy(pClone->m_pHeader, m_pHeader, sizeof(SFPacketHeader));
	memcpy(pClone->m_pPacketBuffer, m_pPacketBuffer, m_packetMaxSize);
	pClone->m_bEncoded = m_bEncoded;
	pClone->m_usCurrentReadPosition = m_usCurrentReadPosition;
	
	return pClone;
}

void SFPacket::Release()
{
	SFCGSFPacketProtocol::DisposePacket(this);
}