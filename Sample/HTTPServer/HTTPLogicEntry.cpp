#include "stdafx.h"
#include "HTTPLogicEntry.h"
#include "SFHTTPPacket.h"
#include <iostream>

HTTPLogicEntry::HTTPLogicEntry(void)
{
}


HTTPLogicEntry::~HTTPLogicEntry(void)
{
}

bool HTTPLogicEntry::Initialize()
{
	return true;
}



bool HTTPLogicEntry::ProcessPacket(BasePacket* pPacket)
{
	SFHTTPPacket* pHttpPacket = (SFHTTPPacket*)pPacket;

	switch (pHttpPacket->GetPacketType())
	{
	case SFPACKET_DATA:	
	{
		http::BufferedRequest& httpRequest = pHttpPacket->GetHTTPRequest();
		std::string szBody = httpRequest.body();
		std::string szMessage = httpRequest.header("msg");

		std::cout << szMessage << std::endl;

		const char szRes[] =
			"HTTP/1.1 200 OK \r\n"
			"Content-Type: text/xml\r\n"
			"Content-Length: 16\r\n"
			"\r\n"
			"<xml>hello</xml>";

		pHttpPacket->m_szRawResponse = szRes;

		SFEngine::GetInstance()->SendRequest(pPacket);
						  
	}
		break;
	}

	return true;
}