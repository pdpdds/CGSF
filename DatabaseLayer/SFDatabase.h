#pragma once
#include "DBStruct.h"
#include "SFDispatch.h"

class BasePacket;

class SFDatabase
{
public:
	SFDatabase(void){}
	virtual ~SFDatabase(void){}

	virtual BOOL Initialize() = 0;
	virtual BOOL RegisterDBService() = 0;

	virtual BasePacket* Alloc() = 0;
	virtual BOOL Call(BasePacket* pMessage) = 0;
	virtual BOOL Release(BasePacket* pMessage) = 0;

	_DBConnectionInfo* GetInfo(){return &m_Info;}
	void SetInfo(_DBConnectionInfo& Info){m_Info = Info;}

	SFDispatch<USHORT, std::tr1::function<BOOL(BasePacket*)>, BasePacket*> m_Dispatch;

protected:

private:
	_DBConnectionInfo m_Info;
};

//LogicEntrySingleton::instance()->ReleaseDBMessage(m_pMessage);