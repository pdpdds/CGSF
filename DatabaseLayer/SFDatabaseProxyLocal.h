#pragma once
#include "SFDatabaseProxy.h"
#include "SFDBManager.h"

template <typename T>
class SFDatabaseProxyLocal : public SFDatabaseProxy
{
public:
	SFDatabaseProxyLocal(void){}
	virtual ~SFDatabaseProxyLocal(void)
	{
		m_DBManager.ShutDown();
		ACE_Thread_Manager::instance()->wait_task(&m_DBManager);
	}

	virtual bool Initialize(DBModuleParams& params) override;
	virtual bool SendDBRequest(BasePacket* pMessage) override;

protected:

private:
	SFDBManager<T> m_DBManager;
};


template <typename T>
bool SFDatabaseProxyLocal<T>::Initialize(DBModuleParams& params)
{
	if (params.workerPoolSize <= 0)
		return FALSE;

	m_DBManager.SetDBParams(params);

	m_DBManager.activate();

	return TRUE;
}

template <typename T>
bool SFDatabaseProxyLocal<T>::SendDBRequest(BasePacket* pMessage)
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