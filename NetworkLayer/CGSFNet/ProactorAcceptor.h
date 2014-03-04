#pragma once
#include <ace/Asynch_Acceptor.h>
#include <ace/INet_Addr.h>
#include "ProactorService.h"

class IProactorAgent;

class ProactorAcceptor : public ACE_Asynch_Acceptor<ProactorService>
{
public:
	ProactorAcceptor( INetworkEngine* pOwner );
	virtual ~ProactorAcceptor(void);

	virtual int validate_connection(const ACE_Asynch_Accept::Result& Result, const ACE_INET_Addr& Remote, const ACE_INET_Addr& Local) override;
	virtual ProactorService*	make_handler() override;

protected:

private:
	INetworkEngine* m_pOwner;
};
