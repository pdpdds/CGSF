#pragma once
#include "SFZdbAdpator.h"

class BasePacket;

class SFZdbAdaptorImpl : public SFZdbAdpator
{
public:
	SFZdbAdaptorImpl();
	virtual ~SFZdbAdaptorImpl();

	virtual BOOL RegisterDBService() override;
	virtual bool RecallDBMsg(BasePacket* pMessage) override;

protected:

private:
	BOOL OnLogin(BasePacket* pMessage);
};

