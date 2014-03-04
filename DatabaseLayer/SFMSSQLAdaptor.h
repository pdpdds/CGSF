#pragma once
#include "SFDBAdaptor.h"
#include "SFMSSQL.h"

class SFMSSQLAdaptor : public SFDBAdaptor
{
public:
	SFMSSQLAdaptor(void);
	virtual ~SFMSSQLAdaptor(void);

	SFMSSQL* GetObject(){return m_pMSSql;}

protected:
	virtual BOOL Initialize(_DBConnectionInfo* pInfo) override;	

private:
	SFMSSQL* m_pMSSql;	
};