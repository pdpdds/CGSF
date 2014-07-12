#ifndef __TRANSPORT_H
#define __TRANSPORT_H

#include <string>

#include "service_err.hpp"

class ITransport
{
public:
	virtual void send(char * buf, int size) = 0;
	virtual void recv(std::string &outBuf) = 0;
	
	virtual ~ITransport() {}

};

class IClientTransport : public ITransport
{
public:
	virtual ~IClientTransport() {}
};

class IServerTransport :public ITransport
{
public:
	
	virtual void bind(const std::string& bindAddress) = 0;
	virtual ITransport* waitForConnect() = 0;
	virtual ~IServerTransport(){}

};
#endif