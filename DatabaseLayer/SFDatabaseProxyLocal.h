#pragma once
#include "SFDatabaseProxy.h"
#include "SFDBManager.h"

template <typename T>
class SFDatabaseProxyLocal : public SFDatabaseProxy
{
public:
	SFDatabaseProxyLocal(void){}
	virtual ~SFDatabaseProxyLocal(void){}

	virtual BOOL Initialize(DBModuleParams& params) override;
	virtual BOOL SendDBRequest(BasePacket* pMessage) override;

protected:

private:
	SFDBManager<T> m_DBManager;
};


template <typename T>
BOOL SFDatabaseProxyLocal<T>::Initialize(DBModuleParams& params)
{
	if (params.workerPoolSize <= 0)
		return FALSE;

	m_DBManager.SetDBParams(params);

	m_DBManager.activate();

	return TRUE;
}

template <typename T>
BOOL SFDatabaseProxyLocal<T>::SendDBRequest(BasePacket* pMessage)
{
	if(NULL == pMessage)
		return FALSE;

	SFDBRequest* pReq = (SFDBRequest*)m_DBManager.GetDBReq();

	if(NULL == pReq)
		return FALSE;

	pReq->AttachDBMessage(pMessage);

	m_DBManager.perform(pReq);

	return TRUE;
}