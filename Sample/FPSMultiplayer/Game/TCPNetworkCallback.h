#pragma once
#include <Windows.h>
#include <stdio.h>
#include "INetworkCallback.h"

class TCPNetworkCallback : public INetworkCallback
{
public:
	TCPNetworkCallback(void);
	virtual ~TCPNetworkCallback(void);

	virtual bool HandleNetworkMessage(int PacketID, BYTE* pBuffer, USHORT Length) override;
	virtual void HandleConnect(int Serial) override;
	virtual void HandleDisconnect(int Serial) override;
};
