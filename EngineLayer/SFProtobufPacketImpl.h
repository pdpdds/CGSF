#pragma once
#include "BasePacket.h"
#include <google/protobuf/stubs/common.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/generated_message_reflection.h>

class SFProtobufPacketImpl : public BasePacket
{
public:
	SFProtobufPacketImpl(int serviceId, google::protobuf::Message* message);
	~SFProtobufPacketImpl(void);
public:
	virtual bool Decode(char* pBuf, unsigned int nSize);
	virtual bool Encode();
	virtual google::protobuf::Message& getStructuredData();
private:
	int serviceId;
	std::string* pEncodedStream;
	google::protobuf::Message* message;
	
public:
	std::string* getEncodedStream() const { return pEncodedStream; }
	int getEncodedStreamSize();
	int getServiceId() const { return serviceId; }
};