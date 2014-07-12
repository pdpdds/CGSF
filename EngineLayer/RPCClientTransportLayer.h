#pragma once

#include "transport.hpp"

class RPCClientTransportLayer : public IClientTransport
{
public:
	RPCClientTransportLayer();
	virtual ~RPCClientTransportLayer();

	virtual void send(char * buf, int size)  override;
	virtual void recv(std::string &outBuf) override;
};

