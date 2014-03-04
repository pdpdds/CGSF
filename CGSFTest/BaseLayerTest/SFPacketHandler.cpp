#include "StdAfx.h"
#include "SFPacketHandler.h"

SFPacketHandler::SFPacketHandler(void)
{
}

SFPacketHandler::~SFPacketHandler(void)
{
}

void SFPacketHandler::OnAuth(BasePacket* packet)
{
	SFProtobufPacket<PacketCore::Auth>* pAuth = (SFProtobufPacket<PacketCore::Auth>*)packet;
	DWORD dwEncryptKey = pAuth->GetData().encryptionkey();

	printf("Encrypt Key : %x\n", dwEncryptKey);
}
