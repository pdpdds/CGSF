#pragma once

#pragma warning( push )
#pragma warning( disable:4100 )

class IUDPNetworkCallback
{
public:
	IUDPNetworkCallback(void){}
	~IUDPNetworkCallback(void){}

	virtual bool HandleUDPNetworkMessage(const unsigned char* pData, unsigned int Length) = 0;
	virtual bool ReportMyIP(unsigned int LocalIP, unsigned short LocalPort, unsigned int ExternalIP, unsigned short ExternalPort){return false;}
};

#pragma warning( pop )