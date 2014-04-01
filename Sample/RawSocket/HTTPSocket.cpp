/** \file HTTPSocket.cpp
 **	\date  2004-04-06
 **	\author grymse@alhem.net
**/
/*
Copyright (C) 2004-2011  Anders Hedstrom

This library is made available under the terms of the GNU GPL, with
the additional exemption that compiling, linking, and/or using OpenSSL 
is allowed.

If you would like to use this library in a closed-source application,
a separate license agreement is available. For information about 
the closed-source license agreement for the C++ sockets library,
please visit http://www.alhem.net/Sockets/license.html and/or
email license@alhem.net.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif
#include "HTTPSocket.h"
#include "Parse.h"
#include "ISocketHandler.h"
#include <stdarg.h>
#include <stdio.h>

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif


HTTPSocket::HTTPSocket(ISocketHandler& h)
:TcpSocket(h)
,m_first(true)
,m_header(true)
,m_http_version("HTTP/1.0")
,m_request(false)
,m_response(false)
,m_body_size_left(0)
,m_b_http_1_1(false)
,m_b_keepalive(false)
,m_b_chunked(false)
,m_chunk_size(0)
,m_chunk_state(0)
,m_header_count(0)
,m_max_header_count(MAX_HTTP_HEADER_COUNT)
{
	SetLineProtocol();
	DisableInputBuffer();
}


HTTPSocket::~HTTPSocket()
{
}


void HTTPSocket::OnRawData(const char *buf,size_t len)
{
	if (!m_header)
	{
		if (m_b_chunked)
		{
			size_t ptr = 0;
			while (ptr < len)
			{
				switch (m_chunk_state)
				{
				case 4:
					while (ptr < len && (m_chunk_line.size() < 2 || m_chunk_line.substr(m_chunk_line.size() - 2) != "\r\n"))
						m_chunk_line += buf[ptr++];
					if (m_chunk_line.size() > 1 && m_chunk_line.substr(m_chunk_line.size() - 2) == "\r\n")
					{
						OnDataComplete();
						// prepare for next request(or response)
						m_b_chunked = false;
						SetLineProtocol( true );
						m_first = true;
						m_header = true;
						m_body_size_left = 0;
						if (len - ptr > 0)
						{
							char tmp[TCP_BUFSIZE_READ];
							memcpy(tmp, buf + ptr, len - ptr);
							tmp[len - ptr] = 0;
							OnRead( tmp, len - ptr );
							ptr = len;
						}
					}
					break;
				case 0:
					while (ptr < len && (m_chunk_line.size() < 2 || m_chunk_line.substr(m_chunk_line.size() - 2) != "\r\n"))
						m_chunk_line += buf[ptr++];
					if (m_chunk_line.size() > 1 && m_chunk_line.substr(m_chunk_line.size() - 2) == "\r\n")
					{
						m_chunk_line.resize(m_chunk_line.size() - 2);
						Parse pa(m_chunk_line, ";");
						std::string size_str = pa.getword();
						m_chunk_size = Utility::hex2unsigned(size_str);
						if (!m_chunk_size)
						{
							m_chunk_state = 4;
							m_chunk_line = "";
						}
						else
						{
							m_chunk_state = 1;
							m_chunk_line = "";
						}
					}
					break;
				case 1:
					{
						size_t left = len - ptr;
						size_t sz = m_chunk_size < left ? m_chunk_size : left;
						OnData(buf + ptr, sz);
						m_chunk_size -= sz;
						ptr += sz;
						if (!m_chunk_size)
						{
							m_chunk_state = 2;
						}
					}
					break;
				case 2: // skip CR
					ptr++;
					m_chunk_state = 3;
					break;
				case 3: // skip LF
					ptr++;
					m_chunk_state = 0;
					break;
				}
			}
		}
		else
		if (!m_b_http_1_1 || !m_b_keepalive)
		{
			OnData(buf, len);
			/*
				request is HTTP/1.0 _or_ HTTP/1.1 and not keep-alive

				This means we destroy the connection after the response has been delivered,
				hence no need to reset all internal state variables for a new incoming
				request.
			*/
			m_body_size_left -= len;
			if (!m_body_size_left)
			{
				OnDataComplete();
			}
		}
		else
		{
			size_t sz = m_body_size_left < len ? m_body_size_left : len;
			OnData(buf, sz);
			m_body_size_left -= sz;
			if (!m_body_size_left)
			{
				OnDataComplete();
				// prepare for next request(or response)
				SetLineProtocol( true );
				m_first = true;
				m_header = true;
				m_body_size_left = 0;
				if (len - sz > 0)
				{
					char tmp[TCP_BUFSIZE_READ];
					memcpy(tmp, buf + sz, len - sz);
					tmp[len - sz] = 0;
					OnRead( tmp, len - sz );
				}
			}
		}
	}
}


void HTTPSocket::OnLine(const std::string& line)
{
	if (m_first)
	{
		Parse pa(line);
		std::string str = pa.getword();
		if (str.size() > 4 && Utility::ToLower(str.substr(0,5)) == "http/") // response
		{
			m_http_version = str;
			m_status = pa.getword();
			m_status_text = pa.getrest();
			m_response = true;
		}
		else // request
		{
			m_method = str;
			m_url = pa.getword();
			size_t spl = m_url.find("?");
			if (spl != std::string::npos)
			{
				m_uri = m_url.substr(0,spl);
				m_query_string = m_url.substr(spl + 1);
			}
			else
			{
				m_uri = m_url;
				m_query_string = "";
			}
			m_http_version = pa.getword();
			m_b_http_1_1 = m_http_version.size() > 4 && m_http_version.substr(4) == "/1.1";
			m_b_keepalive = m_b_http_1_1;
			m_request = true;
		}
		m_first = false;
		OnFirst();
		return;
	}
	if (!line.size())
	{
		if (m_body_size_left || !m_b_http_1_1 || !m_b_keepalive || m_b_chunked)
		{
			SetLineProtocol(false);
			m_header = false;
		}
		OnHeaderComplete();
		if (!m_body_size_left && !m_b_chunked)
		{
			OnDataComplete();
		}
		return;
	}
	Parse pa(line,":");
	std::string key = pa.getword();
	std::string value = pa.getrest();
	OnHeader(key,value);
	if (Utility::ToLower(key) == "content-length")
	{
		m_body_size_left = atol(value.c_str());
	}
	if (m_b_http_1_1 && Utility::ToLower(key) == "connection")
	{
		m_b_keepalive = Utility::ToLower(value) != "close";
	}
	if (Utility::ToLower(key) == "transfer-encoding" && Utility::ToLower(value) == "chunked")
	{
		m_b_chunked = true;
	}
	/* If remote end tells us to keep connection alive, and we're operating
	in http/1.1 mode (not http/1.0 mode), then we mark the socket to be
	retained. */
#ifdef ENABLE_POOL
	if (m_b_http_1_1 && m_b_keepalive)
	{
		SetRetain();
	}
#endif
	if (m_header_count++ > m_max_header_count)
	{
		SetCloseAndDelete();
		Handler().LogError(this, "OnLine", m_header_count, "http header count exceeds builtin limit of (" + Utility::l2string(m_max_header_count) + ")", LOG_LEVEL_FATAL);
	}
}


void HTTPSocket::SendResponse()
{
	std::string msg;
	msg = m_http_version + " " + m_status + " " + m_status_text + "\r\n";
	for (string_m::iterator it = m_response_header.begin(); it != m_response_header.end(); ++it)
	{
		std::string key = (*it).first;
		std::string val = (*it).second;
		msg += key + ": " + val + "\r\n";
	}
	for (std::list<std::pair<std::string, std::string> >::iterator it2 = m_response_header_append.begin(); it2 != m_response_header_append.end(); ++it2)
	{
		msg += it2 -> first + ": " + it2 -> second + "\r\n";
	}
	msg += "\r\n";
	Send( msg );
}


void HTTPSocket::AddResponseHeader(const std::string& header, const char *format, ...)
{
	char slask[8192]; // temporary for vsprintf / vsnprintf
	va_list ap;

	va_start(ap, format);
	vsnprintf(slask, sizeof(slask), format, ap);
	va_end(ap);

	m_response_header[header] = slask;
}


void HTTPSocket::SendRequest()
{
	std::string msg;
	msg = m_method + " " + m_url + " " + m_http_version + "\r\n";
	for (string_m::iterator it = m_response_header.begin(); it != m_response_header.end(); ++it)
	{
		std::string key = (*it).first;
		std::string val = (*it).second;
		msg += key + ": " + val + "\r\n";
	}
	msg += "\r\n";
	Send( msg );
}


std::string HTTPSocket::MyUseragent()
{
	std::string version = "C++Sockets/";
#ifdef _VERSION
	version += _VERSION;
#endif
	return version;
}


void HTTPSocket::Reset()
{
	m_first = true;
	m_header = true;
	m_request = false;
	m_response = false;
	SetLineProtocol(true);
        while (m_response_header.size())
        {
                string_m::iterator it = m_response_header.begin();
                m_response_header.erase(it);
        }
        while (m_response_header_append.size())
        {
                std::list<std::pair<std::string, std::string> >::iterator it = m_response_header_append.begin();
                m_response_header_append.erase(it);
        }
        m_header_count = 0;

}


const std::string& HTTPSocket::GetMethod() const
{
	return m_method;
}


void HTTPSocket::SetMethod(const std::string& x)
{
	m_method = x;
}


const std::string& HTTPSocket::GetUrl() const
{
	return m_url;
}


void HTTPSocket::SetUrl(const std::string& x)
{
	m_url = x;
}


const std::string& HTTPSocket::GetUri() const
{
	return m_uri;
}


const std::string& HTTPSocket::GetQueryString() const
{
	return m_query_string;
}


const std::string& HTTPSocket::GetHttpVersion() const
{
	return m_http_version;
}


const std::string& HTTPSocket::GetStatus() const
{
	return m_status;
}


const std::string& HTTPSocket::GetStatusText() const
{
	return m_status_text;
}


bool HTTPSocket::IsRequest() const
{
	return m_request;
}


bool HTTPSocket::IsResponse() const
{
	return m_response;
}


void HTTPSocket::SetHttpVersion(const std::string& x)
{
	m_http_version = x;
}


void HTTPSocket::SetStatus(const std::string& x)
{
	m_status = x;
}


void HTTPSocket::SetStatusText(const std::string& x)
{
	m_status_text = x;
}


void HTTPSocket::AddResponseHeader(const std::string& x,const std::string& y)
{
	m_response_header[x] = y;
}


void HTTPSocket::AppendResponseHeader(const std::string& x,const std::string& y)
{
	m_response_header_append.push_back(std::pair<std::string, std::string>(x,y));
}


void HTTPSocket::SetUri(const std::string& x)
{
	m_uri = x;
}


void HTTPSocket::url_this(const std::string& url_in,std::string& protocol,std::string& host,port_t& port,std::string& url,std::string& file)
{
	Parse pa(url_in,"/");
	std::string user;
	std::string auth;
	protocol = pa.getword(); // http
	if (!strcasecmp(protocol.c_str(), "https:"))
	{
#ifdef HAVE_OPENSSL
		EnableSSL();
#else
		Handler().LogError(this, "url_this", -1, "SSL not available", LOG_LEVEL_WARNING);
#endif
		port = 443;
	}
	else
	{
		port = 80;
	}
	host = pa.getword();
	size_t pos = host.find("@");
	if (pos != std::string::npos)
	{
		user = host.substr(0, pos);
		host = host.substr(pos + 1);
		if (user.find(":") != std::string::npos)
		{
			AddResponseHeader("Authorization", "Basic " + Utility::base64(user));
		}
	}
	if (strstr(host.c_str(),":"))
	{
		Parse pa(host,":");
		pa.getword(host);
		port = static_cast<port_t>(pa.getvalue());
	}
	url = "/" + pa.getrest();
	{
		Parse pa(url,"/");
		std::string tmp = pa.getword();
		while (tmp.size())
		{
			file = tmp;
			tmp = pa.getword();
		}
	}
} // url_this


bool HTTPSocket::ResponseHeaderIsSet(const std::string& name) const
{
	string_m::const_iterator it = m_response_header.find( name );
	if (it != m_response_header.end())
	{
		return true;
	}
	std::list<std::pair<std::string, std::string> >::const_iterator it2;
	for (it2 = m_response_header_append.begin(); it2 != m_response_header_append.end(); ++it2)
	{
		const std::pair<std::string, std::string>& ref = *it2;
		if (!strcasecmp(ref.first.c_str(), name.c_str()) )
		{
			return true;
		}
	}
	return false;
}


#ifdef SOCKETS_NAMESPACE
}
#endif


