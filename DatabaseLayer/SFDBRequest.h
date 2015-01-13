#pragma once
#include <ACE/Method_Request.h>
#include <ACE/TSS_t.h>
#include "Macro.h"

//extern ACE_TSS<SFDatabaseTSS> DatabaseTSS;
class SFDBWorker;
class BasePacket;

class SFDBRequest : public ACE_Method_Request
{
public:
	SFDBRequest(void){}
	virtual ~SFDBRequest(void){}

	BOOL AttachDBMessage(BasePacket* pMsg)
	{
		m_pMessage = pMsg;
		return TRUE;
	}

	virtual int call(void);

	void SetOwner(SFDBWorker* pWorker){m_pOwner = pWorker;}

protected:

private:
	BasePacket* m_pMessage;
	SFDBWorker* m_pOwner;
};

