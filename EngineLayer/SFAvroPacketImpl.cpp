#include "stdafx.h"
#include "SFAvroPacketImpl.h"
#include "SFAvroProtocol.h"

SFAvroPacketImpl::SFAvroPacketImpl()
{
}


SFAvroPacketImpl::~SFAvroPacketImpl()
{
}

void SFAvroPacketImpl::Release()
{
	SFAvroProtocol::DisposePacket(this);
}

