#pragma once

class IRPCService
{
public:
	IRPCService(){}
	virtual ~IRPCService(){}

	virtual void ProcessRPCService(SFPacket* pSFPacket) = 0;
};
