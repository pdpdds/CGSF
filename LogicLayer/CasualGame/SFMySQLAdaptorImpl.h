#pragma once
#include "SFMySQLAdaptor.h"

class SFMySQL;
class BasePacket;

class SFMySQLAdaptorImpl : public SFMySQLAdaptor
{
public:
	SFMySQLAdaptorImpl(void);
	virtual ~SFMySQLAdaptorImpl(void);

	virtual BOOL Initialize(_DBConnectionInfo* pInfo) override;
	virtual BOOL RegisterDBService() override;

protected:

private:
	SFMySQL* m_pMySql;

	BOOL OnLogin( BasePacket* pMessage );
};