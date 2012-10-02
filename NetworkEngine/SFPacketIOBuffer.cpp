#include "StdAfx.h"
#include "SFPacketIOBuffer.h"

SFPacketIOBuffer::SFPacketIOBuffer(void)
{
}

SFPacketIOBuffer::~SFPacketIOBuffer(void)
{
}

BOOL SFPacketIOBuffer::GetPacket(SFPacket* pPacket, int& ErrorCode)
{
	ErrorCode = PACKETIO_ERROR_NONE;

	USHORT HeaderSize = pPacket->GetHeaderSize();

	if(GetUsedBufferSize() < HeaderSize)
	{
		return FALSE;
	}

	if(HeaderSize != GetData((char*)pPacket->GetHeader(), HeaderSize))
	{
		ErrorCode = PACKETIO_ERROR_HEADER;
	}

	int PacketSize = pPacket->GetPacketSize();

	int DataSize = PacketSize - HeaderSize;

	if(DataSize > MAX_PACKET_DATA)
	{
		ErrorCode = PACKETIO_ERROR_DATA;
		return FALSE;
	}

	if(GetUsedBufferSize() < DataSize)
	{
		return FALSE;
	}

	int GetDataSize = GetData((char*)pPacket->GetDataBuffer(), DataSize);

	if(DataSize != GetDataSize)
	{
		ErrorCode = PACKETIO_ERROR_DATA;
		return FALSE;
	}

	return TRUE;
}
