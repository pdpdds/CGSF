#pragma once

class IRPCInterface
{
public:
	IRPCInterface(){}
	virtual ~IRPCInterface(){}

	virtual void ProcessRPCService(SFPacket* pSFPacket) = 0;
};
