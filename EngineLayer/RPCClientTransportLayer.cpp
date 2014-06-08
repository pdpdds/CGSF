#include "stdafx.h"
#include "RPCClientTransportLayer.h"
#include "SFPacket.h"
#include "SFNetworkEntry.h"
#include "SFTCPNetwork.h"

RPCClientTransportLayer::RPCClientTransportLayer()
{
}

RPCClientTransportLayer::~RPCClientTransportLayer()
{
}

void RPCClientTransportLayer::send(char * buf, int size) throw(ServiceException)
{
	SFPacket packet;
	packet.SetPacketID(60000);
	packet.Write((const BYTE*)buf, size);
	SFNetworkEntry::GetInstance()->TCPSend(&packet);
}
void RPCClientTransportLayer::recv(std::string &outBuf) throw(ServiceException)
{
	SFPacket* pPacket = NULL;

	DWORD dwCurrentTime = GetTickCount();
	DWORD dwLastTime = dwCurrentTime;

	while (pPacket == NULL)
	{
		pPacket = (SFPacket*)SFNetworkEntry::GetInstance()->GetTCPNetwork()->GetRPCResult();

		//if (dwLastTime - dwCurrentTime > 5000)
			//break;

		//dwLastTime = GetTickCount();

		Sleep(1);
	}

	if (pPacket != NULL)
	{
		int size = pPacket->GetDataSize();
		outBuf.append((const char*)pPacket->GetData(), pPacket->GetDataSize());
		SFEngine::GetInstance()->ReleasePacket(pPacket);
	}
}