#pragma once
#include "SFDataBaseProxy.h"

class SFDatabaseProxyImpl : public SFDatabaseProxy
{
public:
	SFDatabaseProxyImpl(SFDatabaseProxy* pProxy){m_pDataBaseProxy = pProxy;}
	virtual ~SFDatabaseProxyImpl(void)
	{ 
		if (m_pDataBaseProxy)
			delete m_pDataBaseProxy;
	}

	virtual BOOL Initialize(DBModuleParams& params)
	{
		return m_pDataBaseProxy->Initialize(params);
	}

	virtual BOOL SendDBRequest(BasePacket* pMessage)
	{
		return m_pDataBaseProxy->SendDBRequest(pMessage);
	}

protected:

private:
	SFDatabaseProxy* m_pDataBaseProxy;
};
