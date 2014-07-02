#include "stdafx.h"
#include "CommunityManager.h"

#pragma warning (disable : 4189) 
#pragma warning (disable : 4100) 

CommunityManager::CommunityManager()
{
	m_pContext = new zmq::context_t(1);
	m_pSocket = new zmq::socket_t(*m_pContext, ZMQ_REQ);
}

CommunityManager::~CommunityManager()
{
}

bool CommunityManager::Connect(char* szService, char* szIP, unsigned short port)
{
	
	std::string szAddr = "tcp://";
	szAddr += szIP;
	szAddr += ':';

	char buf[256] = { 0, };
	itoa(port, buf, 10);
	szAddr += buf;

	m_pSocket->connect(szAddr.c_str());

	return true;
}

bool CommunityManager::SendAndRecv(char* szService, zmq::message_t& req, zmq::message_t& res)
{
	//zmq::message_t request(length);
	//memcpy((void *)request.data(), pBuffer, length);
	m_pSocket->send(req);

	m_pSocket->recv(&res);

	return true;
}