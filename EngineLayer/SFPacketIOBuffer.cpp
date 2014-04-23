#include "StdAfx.h"
#include "SFPacketIOBuffer.h"

SFPacketIOBuffer::SFPacketIOBuffer(void)
{
}

SFPacketIOBuffer::~SFPacketIOBuffer(void)
{
}

bool SFPacketIOBuffer::GetPacket(SFPacketHeader& header, char* pBuffer, int& errorCode)
{
	errorCode = PACKETIO_ERROR_NONE;

	USHORT headerSize = sizeof(SFPacketHeader);

	if (GetUsedBufferSize() < headerSize)
	{
		return false;
	}

	if (headerSize != GetData((char*)&header, headerSize))
	{
		errorCode = PACKETIO_ERROR_HEADER;
	}

	int dataSize = header.dataSize;

	if (dataSize > MAX_PACKET_DATA)
	{
		errorCode = PACKETIO_ERROR_DATA;
		return false;
	}

	if (GetUsedBufferSize() < dataSize)
	{
		return false;
	}

	int getDataSize = GetData((char*)pBuffer, dataSize);

	if (dataSize != getDataSize)
	{
		errorCode = PACKETIO_ERROR_DATA;
		return false;
	}

	return true;
}
