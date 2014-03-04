#include "ProactorAcceptor.h"
#include "SingltonObject.h"

ProactorAcceptor::ProactorAcceptor( INetworkEngine* pOwner )
: m_pOwner(pOwner)
{
	ProactorServiceMapSingleton::instance();
	//ACEAllocator* AceMemoryPool = new ACEAllocator(100, sizeof(SFProactorService));
}

ProactorAcceptor::~ProactorAcceptor(void)
{
}

int ProactorAcceptor::validate_connection( const ACE_Asynch_Accept::Result& Result, const ACE_INET_Addr& Remote, const ACE_INET_Addr& Local )
{
	struct in_addr* remote_addr = reinterpret_cast<struct in_addr*>(Remote.get_addr());
	struct in_addr* local_addr = reinterpret_cast <struct in_addr*>(Local.get_addr());

	ACE_UNUSED_ARG(Result);
	ACE_UNUSED_ARG(Remote);
	ACE_UNUSED_ARG(Local);

	return 0;
}

ProactorService* ProactorAcceptor::make_handler()
{
	ProactorService* pProactorService = new ProactorService();
	pProactorService->SetOwner(m_pOwner);
	
	return pProactorService;
}