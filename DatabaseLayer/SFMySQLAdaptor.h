#pragma once
#include "SFDBAdaptor.h"

class SFMySQL;
class BasePacket;

class SFMySQLAdaptor : public SFDBAdaptor
{
public:
	SFMySQLAdaptor(void);
	virtual ~SFMySQLAdaptor(void);

	virtual BOOL Initialize(_DBConnectionInfo* pInfo) override;	

protected:

private:
	SFMySQL* m_pMySql;	
};