#pragma once
#include <d3dx9math.h>
#include "IUDPNetworkCallback.h"
#include "GamePacketStructure.h"

class UDPNetworkCallback : public IUDPNetworkCallback
{
public:
	UDPNetworkCallback(void);
	virtual ~UDPNetworkCallback(void);

	virtual bool HandleUDPNetworkMessage(const unsigned char* pData, unsigned int Length) override;
	virtual bool ReportMyIP(unsigned int LocalIP, unsigned short LocalPort, unsigned int ExternalIP, unsigned short ExternalPort) override;

protected:

private:
};
