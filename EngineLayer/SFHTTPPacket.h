#pragma once
#include "http.hpp"

class SFHTTPPacket : public BasePacket
{
public:
	SFHTTPPacket();
	virtual ~SFHTTPPacket();

	virtual void Release() override;
	virtual BasePacket* Clone() override;

	http::BufferedRequest& GetHTTPRequest(){return m_request;}

	http::BufferedRequest m_request;
	std::string m_szRawResponse;
};

