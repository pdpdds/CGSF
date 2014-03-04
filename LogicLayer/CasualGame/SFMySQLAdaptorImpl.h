#pragma once
#include "SFMySQLAdaptor.h"

class BasePacket;

class SFMySQLAdaptorImpl : public SFMySQLAdaptor
{
public:
	SFMySQLAdaptorImpl(void);
	virtual ~SFMySQLAdaptorImpl(void);

	virtual BOOL RegisterDBService() override;

protected:

private:
	BOOL OnLogin( BasePacket* pMessage );
};