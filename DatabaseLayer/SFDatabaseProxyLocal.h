#pragma once
#include "SFDatabaseProxy.h"
#include "SFDBManager.h"

template <typename T>
class SFDatabaseProxyLocal : public SFDatabaseProxy
{
public:
	SFDatabaseProxyLocal(void){}
	virtual ~SFDatabaseProxyLocal(void){}

	virtual BOOL Initialize(int workerPoolSize = -1) override;
	virtual BOOL SendDBRequest(BasePacket* pMessage) override;

protected:

private:
	SFDBManager<T> m_DBManager;
};


template <typename T>
BOOL SFDatabaseProxyLocal<T>::Initialize(int workerPoolSize = -1)
{
	if(workerPoolSize > 0)
		m_DBManager.SetWorkerPoolSize(workerPoolSize);

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