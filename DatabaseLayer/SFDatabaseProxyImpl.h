#pragma once
#include "SFDataBaseProxy.h"

class SFDatabaseProxyImpl : public SFDatabaseProxy
{
public:
	SFDatabaseProxyImpl(SFDatabaseProxy* pProxy){m_pDataBaseProxy = pProxy;}
	virtual ~SFDatabaseProxyImpl(void){}

	virtual BOOL Initialize()
	{
		return m_pDataBaseProxy->Initialize();
	}

	virtual BOOL SendDBRequest(BasePacket* pMessage)
	{
		return m_pDataBaseProxy->SendDBRequest(pMessage);
	}

protected:

private:
	SFDatabaseProxy* m_pDataBaseProxy;
};
