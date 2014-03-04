#pragma once
#include "SFDispatch.h"
#include "BasePacket.H"

class SFDBAdaptor
{
public:
	SFDBAdaptor(void);
	virtual ~SFDBAdaptor(void);

	virtual BOOL Initialize(_DBConnectionInfo* pInfo) = 0;
	virtual BOOL RegisterDBService() = 0;

	BOOL Call(BasePacket* pMessage)
	{
		return m_Dispatch.HandleMessage(pMessage->GetPacketID(), pMessage);

		return FALSE;
	}

protected:
	SFDispatch<USHORT, std::tr1::function<BOOL(BasePacket*)>, BasePacket*> m_Dispatch;

private:
};

