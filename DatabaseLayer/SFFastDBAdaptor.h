#pragma once
#include "SFDBAdaptor.h"
#include "SFFastDB.h"

class SFFastDBAdaptor : public SFDBAdaptor
{
public:
	SFFastDBAdaptor(void);
	virtual ~SFFastDBAdaptor(void);

	SFFastDB* GetObject(){return m_pFastDB;}

protected:
	virtual BOOL Initialize(_DBConnectionInfo* pInfo) override;	

private:
	SFFastDB* m_pFastDB;	

};

