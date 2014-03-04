#pragma once
#include "SFDBAdaptor.h"

class SFMySQL;

class SFMySQLAdaptor : public SFDBAdaptor
{
public:
	SFMySQLAdaptor(void);
	virtual ~SFMySQLAdaptor(void);

	SFMySQL* GetObject(){return m_pMySql;}

protected:
	virtual BOOL Initialize(_DBConnectionInfo* pInfo) override;	

private:
	SFMySQL* m_pMySql;	
};