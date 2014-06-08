#ifndef __TRANSPORT_H
#define __TRANSPORT_H

#include <string>

#include "service_err.hpp"

class ITransport
{
public:
	virtual void send(char * buf, int size) throw(ServiceException) = 0;
	virtual void recv(std::string &outBuf) throw(ServiceException) = 0;
	//virtual void close() throw(ServiceException) = 0;
	//virtual int getLastErrorCode() = 0;
	virtual ~ITransport() {}

};

class IClientTransport : public ITransport
{
public:
	// client side
	//virtual void connect(const std::string& serverAddress) throw(ServiceException) = 0;
	virtual ~IClientTransport() {}
};

class IServerTransport :public ITransport
{
public:
	// server side
	virtual void bind(const std::string& bindAddress) throw(ServiceException) = 0;
	virtual ITransport* waitForConnect() throw(ServiceException) = 0;
	virtual ~IServerTransport(){}

};
#endif