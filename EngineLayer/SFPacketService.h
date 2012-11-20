#pragma once
#include "IPacketService.h"
#include "SFUserSession.h"

class SFPacketService : public IPacketService
{
public:
	SFPacketService(void);
	virtual ~SFPacketService(void);

	virtual bool OnData(int Serial, char* pData, unsigned short Length) override;

protected:

private:
	SFUserSession m_UserSession;
};

