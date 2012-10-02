#include "StdAfx.h"
#include "SFProactorAcceptor.h"

SFProactorAcceptor::SFProactorAcceptor( IProactorAgent* pOwner )
: m_pOwner(pOwner)
{

}

SFProactorAcceptor::~SFProactorAcceptor(void)
{
}

int SFProactorAcceptor::validate_connection( const ACE_Asynch_Accept::Result& Result, const ACE_INET_Addr& Remote, const ACE_INET_Addr& Local )
{
	struct in_addr* remote_addr = reinterpret_cast<struct in_addr*>(Remote.get_addr());
	struct in_addr* local_addr = reinterpret_cast <struct in_addr*>(Local.get_addr());

	ACE_UNUSED_ARG(Result);
	ACE_UNUSED_ARG(Remote);
	ACE_UNUSED_ARG(Local);

	return 0;
}

SFProactorService* SFProactorAcceptor::make_handler()
{
	SFProactorService* pProactiveService = new SFProactorService();
	pProactiveService->SetOwner(m_pOwner);
	
	return pProactiveService;
}