#pragma once
#include "INetworkCallback.h"

class TCPCallback : public INetworkCallback
{
public:
	TCPCallback(void);
	virtual ~TCPCallback(void);

	virtual bool HandleNetworkMessage(int PacketID, BYTE* pBuffer, USHORT Length) override;
};

