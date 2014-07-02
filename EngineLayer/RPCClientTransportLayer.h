#pragma once

#include "transport.hpp"

class RPCClientTransportLayer : public IClientTransport
{
public:
	RPCClientTransportLayer();
	virtual ~RPCClientTransportLayer();

	virtual void send(char * buf, int size) throw(ServiceException) override;
	virtual void recv(std::string &outBuf) throw(ServiceException) override;
};

