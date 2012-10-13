#pragma once
#include "SFDatabaseProxy.h"
#include "SFDBManager.h"

class SFDatabaseProxyLocal : public SFDatabaseProxy
{
public:
	SFDatabaseProxyLocal(void);
	virtual ~SFDatabaseProxyLocal(void);

	virtual BOOL Initialize() override;
	virtual BOOL SendDBRequest(SFMessage* pMessage) override;

protected:

private:
	SFDBManager m_DBManager;
};