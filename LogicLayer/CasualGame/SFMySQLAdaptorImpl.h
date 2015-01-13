#pragma once
#include "SFMySQLAdaptor.h"

class BasePacket;

class SFMySQLAdaptorImpl : public SFMySQLAdaptor
{
public:
	SFMySQLAdaptorImpl(void);
	virtual ~SFMySQLAdaptorImpl(void);

	virtual BOOL RegisterDBService() override;
	virtual bool RecallDBMsg(BasePacket* pMessage) override;

protected:

private:
	BOOL OnLogin( BasePacket* pMessage );
};