#pragma once
#pragma warning( push )
#pragma warning (disable : 4189) 
#pragma warning (disable : 4100)
#pragma warning (disable : 4267) 
#include <zmq.hpp>
#pragma warning( pop )
#include <string>
#include <iostream>


class CommunityManager
{
public:
	CommunityManager();
	virtual ~CommunityManager();
	bool Connect(char* szService, char* szIP, unsigned short port);
	bool SendAndRecv(char* szService, zmq::message_t& req, zmq::message_t& res);
	bool Open(char* szService);

	zmq::context_t* m_pContext;
	zmq::socket_t* m_pSocket;
};