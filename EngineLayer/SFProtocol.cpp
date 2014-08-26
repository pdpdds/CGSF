#pragma warning(disable : 4244)


#include "stdafx.h"
#include "SFProtocol.h"

SFProtocol::SFProtocol(void)
{
}

SFProtocol::~SFProtocol(void)
{
}

int SFProtocol::serializeOutgoingPacket( BasePacket& packet, DataBuffer& Buffer,  unsigned int& nWrittenBytes )
{
	int nRet = encodeOutgoingPacket(packet);
	if(nRet != Success)
		return nRet;
	return frameOutgoingPacket(packet, Buffer, nWrittenBytes);
}

int SFProtocol::tryDeserializeIncomingPacket( DataBuffer& Buffer, BasePacket*& pPacket, int& serviceId, unsigned int& nExtractedBytes )
{
	int nRet = tryDeframeIncomingPacket(Buffer, pPacket, serviceId, nExtractedBytes);
	if(nRet != Success)
		return nRet;

	return decodeIncomingPacket(pPacket, serviceId);
}