#pragma once
#include <EngineInterface/ISessionService.h>
#include <EngineInterface/IPacketProtocol.h>

class SFSessionService : public ISessionService
{
public:
	SFSessionService(IPacketProtocol* pProtocol) : ISessionService(pProtocol){}
	virtual ~SFSessionService(void){}

	virtual bool OnReceive(char* pData, unsigned short Length, bool bServerObject) override;
	//virtual bool SendRequest(ISession *pSession, BasePacket* pPacket) override;

protected:

private:
	
};