#pragma once
#include "ACEHeader.h"
#include <EngineInterface\/INetworkEngine.h>

class ACETimeOutHandler : public ACE_Handler
{
public:
	ACETimeOutHandler(INetworkEngine* pOwner){m_pOwner = pOwner;}
	virtual ~ACETimeOutHandler(void){}

	 virtual void handle_time_out (const ACE_Time_Value &tv,
                                const void *arg)
    {
		m_pOwner->GetEngine()->OnTimer(arg);
    }

protected:

private:
	INetworkEngine* m_pOwner;
};

