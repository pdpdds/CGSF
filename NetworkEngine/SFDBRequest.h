#pragma once
#include <ACE/Method_Request.h>
#include <ACE/TSS_t.h>

void SendToLogic(SFMessage* pMessage);

class SFMessage;

class SFDBRequest : public ACE_Method_Request
{
public:
	SFDBRequest(void);
	virtual ~SFDBRequest(void);

	BOOL AttachDBMessage(SFMessage* pMsg)
	{
		m_pMessage = pMsg;
		return TRUE;
	}

	virtual int call(void);

protected:

private:
	SFMessage* m_pMessage;
};