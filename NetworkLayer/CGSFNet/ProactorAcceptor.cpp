#include "ProactorAcceptor.h"
#include "SingltonObject.h"

ProactorAcceptor::ProactorAcceptor( INetworkEngine* pOwner )
: m_pOwner(pOwner)
, m_acceptorNum(-1)
{
	ProactorServiceManagerSinglton::instance();
	//ACEAllocator* AceMemoryPool = new ACEAllocator(100, sizeof(SFProactorService));
}

ProactorAcceptor::~ProactorAcceptor(void)
{
}

int ProactorAcceptor::validate_connection( const ACE_Asynch_Accept::Result& result, const ACE_INET_Addr& remote, const ACE_INET_Addr& local )
{
	struct in_addr* remote_addr = reinterpret_cast<struct in_addr*>(remote.get_addr());
	struct in_addr* local_addr = reinterpret_cast <struct in_addr*>(local.get_addr());

	ACE_UNUSED_ARG(result);
	ACE_UNUSED_ARG(remote);
	ACE_UNUSED_ARG(local);

	return 0;
}

ProactorService* ProactorAcceptor::make_handler()
{
	ProactorService* pProactorService = new ProactorService(m_acceptorNum);
	pProactorService->SetOwner(m_pOwner);
	
	return pProactorService;
}