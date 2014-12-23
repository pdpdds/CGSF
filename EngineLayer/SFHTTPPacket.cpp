#include "stdafx.h"
#include "SFHTTPPacket.h"
#include "SFHTTPProtocol.h"


SFHTTPPacket::SFHTTPPacket()
{
}


SFHTTPPacket::~SFHTTPPacket()
{
}

void SFHTTPPacket::Release()
{
	SFHTTPProtocol::DisposePacket(this);
}

BasePacket* SFHTTPPacket::Clone()
{
	return NULL;
}