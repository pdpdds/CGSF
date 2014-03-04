#pragma once
#include "SFDataBaseProxy.h"

class SFDatabaseProxyImpl : public SFDatabaseProxy
{
public:
	SFDatabaseProxyImpl(SFDatabaseProxy* pProxy){m_pDataBaseProxy = pProxy;}
	virtual ~SFDatabaseProxyImpl(void){}

	virtual BOOL Initialize(int workerPoolSize = -1)
	{
		return m_pDataBaseProxy->Initialize(workerPoolSize);
	}

	virtual BOOL SendDBRequest(BasePacket* pMessage)
	{
		return m_pDataBaseProxy->SendDBRequest(pMessage);
	}

protected:

private:
	SFDatabaseProxy* m_pDataBaseProxy;
};
