#pragma once
#include "SFDBAdaptor.h"

class SFZdb;

class SFZdbAdpator : public SFDBAdaptor
{
public:
	SFZdbAdpator();
	virtual ~SFZdbAdpator();

	SFZdb* GetObject(){ return m_pZdb; }

protected:
	virtual BOOL Initialize(_DBConnectionInfo* pInfo) override;

private:
	SFZdb* m_pZdb;
};

