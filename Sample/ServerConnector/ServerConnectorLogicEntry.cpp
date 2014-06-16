#include "stdafx.h"
#include "ServerConnectorLogicEntry.h"
#include "SFJsonPacket.h"
#include <iostream>

ServerConnectorLogicEntry::ServerConnectorLogicEntry(void)
{
}


ServerConnectorLogicEntry::~ServerConnectorLogicEntry(void)
{
}

bool ServerConnectorLogicEntry::Initialize()
{
	return true;
}

bool ServerConnectorLogicEntry::ProcessPacket(BasePacket* pPacket)
{
	switch (pPacket->GetPacketType())
	{	
	case SFPACKET_DATA:		
		SFJsonPacket* pJsonPacket = (SFJsonPacket*)pPacket;

		std::cout << "Received : " << pJsonPacket->GetData().GetValue<tstring>("ECHO") << std::endl;

		break;
	}

	return true;
}

