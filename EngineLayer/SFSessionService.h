#pragma once
#include "ISessionService.h"
#include "IPacketProtocol.h"

class SFSessionService : public ISessionService
{
public:
	SFSessionService(IPacketProtocol* pProtocol) : ISessionService(pProtocol){}
	virtual ~SFSessionService(void){}

	virtual bool OnReceive(char* pData, unsigned short Length) override;
	virtual bool SendRequest(ISession *pSession, BasePacket* pPacket) override;

protected:

private:
	
};