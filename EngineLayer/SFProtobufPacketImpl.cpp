#pragma warning(disable : 4244)
#pragma warning(disable : 4267)

#include "stdafx.h"
#include "SFProtobufPacketImpl.h"
#include "SFProtobufProtocol.h"

#define nReceiveBufferSize 8096
SFProtobufPacketImpl::SFProtobufPacketImpl( int serviceId, google::protobuf::Message* message )
: extendedData(nReceiveBufferSize)
{
	this->serviceId = serviceId;
	this->message = message;
	pEncodedStream = NULL;
}

SFProtobufPacketImpl::~SFProtobufPacketImpl( void )
{
	delete pEncodedStream;
}

bool SFProtobufPacketImpl::Decode( char* pBuf, unsigned int nSize )
{
	if(nSize == 0)
		return true;
	return getStructuredData().ParseFromArray(pBuf, nSize);
}

bool SFProtobufPacketImpl::Encode()
{
	if(pEncodedStream)
		return true;
	pEncodedStream = new std::string();
	if(!getStructuredData().SerializeToString(pEncodedStream))
	{
		delete pEncodedStream;
		pEncodedStream = NULL;
		return false;
	}

	getStructuredData().ByteSize();
	return true;
}

int SFProtobufPacketImpl::getEncodedStreamSize()
{
	return getStructuredData().GetCachedSize();
}

google::protobuf::Message& SFProtobufPacketImpl::getStructuredData()
{
	return (*message);
}

void SFProtobufPacketImpl::Release()
{
	SFProtobufProtocol::DisposePacket(this);
}