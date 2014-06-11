#pragma once
#include "interface/IUDPNetworkCallback.h"

class UDPCallback : public IUDPNetworkCallback
{
public:
	UDPCallback(void);
	virtual ~UDPCallback(void);

	virtual bool HandleUDPNetworkMessage(const unsigned char* pData, unsigned int Length) override;
	virtual bool ReportMyIP(unsigned int LocalIP, unsigned short LocalPort, unsigned int ExternalIP, unsigned short ExternalPort) override;

protected:

private:
};