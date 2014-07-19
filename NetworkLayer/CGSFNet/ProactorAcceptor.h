#pragma once
#include <ace/Asynch_Acceptor.h>
#include <ace/INet_Addr.h>
#include "ProactorService.h"
#include <string>

class IProactorAgent;

class ProactorAcceptor : public ACE_Asynch_Acceptor<ProactorService>
{
public:
	ProactorAcceptor( INetworkEngine* pOwner, char* szIP, unsigned short port );
	virtual ~ProactorAcceptor(void);

	virtual int validate_connection(const ACE_Asynch_Accept::Result& Result, const ACE_INET_Addr& Remote, const ACE_INET_Addr& Local) override;
	virtual ProactorService*	make_handler() override;

	void SetAcceptorNum(int acceptorNum){ m_acceptorNum = acceptorNum; }
	unsigned short GetPort(){ return m_port; }

protected:

private:
	INetworkEngine* m_pOwner;
	int m_acceptorNum;
	std::string m_szIP;
	unsigned short m_port;
};
