#pragma once
#include <ace/Asynch_Acceptor.h>
#include <ace/INet_Addr.h>
#include "SFProactorService.h"

class IProactorAgent;

class SFProactorAcceptor : public ACE_Asynch_Acceptor<SFProactorService>
{
public:
	SFProactorAcceptor(IProactorAgent* pOwner);
	virtual ~SFProactorAcceptor(void);

	virtual int validate_connection(const ACE_Asynch_Accept::Result& Result, const ACE_INET_Addr& Remote, const ACE_INET_Addr& Local) override;
	virtual SFProactorService*	make_handler() override;

protected:

private:
	IProactorAgent* m_pOwner;
};
