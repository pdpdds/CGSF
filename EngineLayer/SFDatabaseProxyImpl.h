#pragma once
#include "SFDataBaseProxy.h"

class SFDatabaseProxyImpl : public SFDatabaseProxy
{
public:
	SFDatabaseProxyImpl(SFDatabaseProxy* pProxy);
	virtual ~SFDatabaseProxyImpl(void);

	virtual BOOL Initialize() override;
	virtual BOOL SendDBRequest(SFMessage* pMessage) override;

protected:

private:
	SFDatabaseProxy* m_pDataBaseProxy;
};
