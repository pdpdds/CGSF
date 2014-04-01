/**
 **	\file HttpRequest.cpp
 **	\date  2007-10-05
 **	\author grymse@alhem.net
**/
/*
Copyright (C) 2007-2011  Anders Hedstrom

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
#include "HttpRequest.h"
#include "MemFile.h"
#include "HttpdForm.h"
#include "HttpdCookies.h"
#include "Parse.h"
#include "Exception.h"
#ifdef MACOSX
#include <crt_externs.h>
#define environ (*_NSGetEnviron())
#endif

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif

#ifdef _DEBUG
#define DEB(x) x; fflush(stderr);
#else
#define DEB(x)
#endif


// --------------------------------------------------------------------------------------
HttpRequest::HttpRequest() : HttpTransaction()
, m_server_port(0)
, m_is_ssl(false)
, m_body_file(NULL)
, m_form(NULL)
{
}


// --------------------------------------------------------------------------------------
#ifndef _WIN32
HttpRequest::HttpRequest(FILE *fil) : HttpTransaction()
, m_server_port(0)
, m_is_ssl(false)
, m_body_file(NULL)
, m_form(NULL)
{
	int i = 0;
DEB(	std::cout << "Initialize HttpRequest from cgi...\n";)
	while (environ[i] && *environ[i])
	{
		Parse pa(environ[i], "=");
		std::string key = pa.getword();
		std::string value = pa.getrest();
		if (key == "REQUEST_METHOD")
			m_method = value;
		else
		if (key == "SERVER_PROTOCOL")
			m_protocol = value;
		else
		if (key == "PATH_INFO")
			m_req_uri = value;
		else
		if (key == "REMOTE_ADDR")
			m_remote_addr = value;
		else
		if (key == "REMOTE_HOST")
			m_remote_host = value;
		else
		if (key == "SERVER_NAME")
			m_server_name = value;
		else
		if (key == "SERVER_PORT")
			m_server_port = atoi(value.c_str());
		else
		if (key.size() > 5 && key.substr(0, 5) == "HTTP_")
		{
			key = key.substr(5);
			for (size_t pos = 0; pos < key.size(); pos++)
			{
				if (key[pos] == '_')
					key[pos] = '-';
				else
				if (key[pos] >= 'A' && key[pos] <= 'Z')
					key[pos] |= 32;
			}
DEB(			std::cout << " http header '" << key << "' == '" << value << "\n";)
			SetHeader(key, value);
		}
		++i;
	}
DEB(	std::cout << " setup http form\n";)
	m_form = std::auto_ptr<HttpdForm>(new HttpdForm(fil));
}
#endif


// --------------------------------------------------------------------------------------
HttpRequest::HttpRequest(const HttpRequest& src) : HttpTransaction(src)
, m_method(src.m_method)
, m_protocol(src.m_protocol)
, m_req_uri(src.m_req_uri)
, m_remote_addr(src.m_remote_addr)
, m_remote_host(src.m_remote_host)
, m_server_name(src.m_server_name)
, m_server_port(src.m_server_port)
, m_is_ssl(src.m_is_ssl)
, m_attribute(src.m_attribute)
, m_null(src.m_null)
, m_body_file(src.m_body_file)
, m_form(src.m_form)
, m_cookies(src.m_cookies)
, m_cookie(src.m_cookie)
{
}


// --------------------------------------------------------------------------------------
HttpRequest::~HttpRequest()
{
}


// --------------------------------------------------------------------------------------
HttpRequest& HttpRequest::operator=(const HttpRequest& src)
{
	m_method = src.m_method;
	m_protocol = src.m_protocol;
	m_req_uri = src.m_req_uri;
	m_remote_addr = src.m_remote_addr;
	m_remote_host = src.m_remote_host;
	m_server_name = src.m_server_name;
	m_server_port = src.m_server_port;
	m_is_ssl = src.m_is_ssl;
	m_attribute = src.m_attribute;
	m_null = src.m_null;
	m_body_file = src.m_body_file;
	m_form = src.m_form;
	m_cookies = src.m_cookies;
	m_cookie = src.m_cookie;

	HttpTransaction::operator=(src);

	return *this;
}


// --------------------------------------------------------------------------------------
void HttpRequest::SetHttpMethod(const std::string& value)
{
	m_method = value;
}


const std::string& HttpRequest::HttpMethod() const
{
	return m_method;
}



// --------------------------------------------------------------------------------------
void HttpRequest::SetHttpVersion(const std::string& value)
{
	m_protocol = value;
}


const std::string& HttpRequest::HttpVersion() const
{
	return m_protocol;
}



// --------------------------------------------------------------------------------------
void HttpRequest::SetUri(const std::string& value)
{
	m_req_uri = value;
}


const std::string& HttpRequest::Uri() const
{
	return m_req_uri;
}



// --------------------------------------------------------------------------------------
void HttpRequest::SetRemoteAddr(const std::string& value)
{
	m_remote_addr = value;
}


const std::string& HttpRequest::RemoteAddr() const
{
	return m_remote_addr;
}



// --------------------------------------------------------------------------------------
void HttpRequest::SetRemoteHost(const std::string& value)
{
	m_remote_host = value;
}


const std::string& HttpRequest::RemoteHost() const
{
	return m_remote_host;
}



// --------------------------------------------------------------------------------------
void HttpRequest::SetServerName(const std::string& value)
{
	m_server_name = value;
}


const std::string& HttpRequest::ServerName() const
{
	return m_server_name;
}



// --------------------------------------------------------------------------------------
void HttpRequest::SetServerPort(int value)
{
	m_server_port = value;
}


int HttpRequest::ServerPort() const
{
	return m_server_port;
}



// --------------------------------------------------------------------------------------
void HttpRequest::SetIsSsl(bool value)
{
	m_is_ssl = value;
}


bool HttpRequest::IsSsl() const
{
	return m_is_ssl;
}



// --------------------------------------------------------------------------------------
void HttpRequest::SetAttribute(const std::string& key, const std::string& value)
{
	m_attribute[key] = value;
}


void HttpRequest::SetAttribute(const std::string& key, long value)
{
	m_attribute[key] = Utility::l2string(value);
}


const std::string& HttpRequest::Attribute(const std::string& key) const
{
	Utility::ncmap<std::string>::const_iterator it;
	if ( (it = m_attribute.find(key)) != m_attribute.end())
		return it -> second;
	return m_null;
}


// --------------------------------------------------------------------------------------
const Utility::ncmap<std::string>& HttpRequest::Attributes() const
{
	return m_attribute;
}


// --------------------------------------------------------------------------------------
void HttpRequest::AddCookie(const std::string& str)
{
	m_cookies.add( str );
	Parse pa(str, ";");
	std::string lstr = pa.getword();
	while (!lstr.empty())
	{
		Parse pa2(lstr, "=");
		std::string name = pa2.getword();
		m_cookie[name] = lstr;
DEB(fprintf(stderr, " *** AddCookie '%s' = '%s'\n", name.c_str(), lstr.c_str());)
		lstr = pa.getword();
	}
}


// --------------------------------------------------------------------------------------
void HttpRequest::InitBody( size_t sz )
{
	if (!m_body_file.get())
		m_body_file = std::auto_ptr<IFile>(new MemFile);
DEB(	else
		fprintf(stderr, "Body data file already opened\n");)
}


// --------------------------------------------------------------------------------------
void HttpRequest::Write( const char *buf, size_t sz )
{
	if (m_body_file.get())
		m_body_file -> fwrite(buf, 1, sz);
DEB(	else
		fprintf(stderr, "Write: Body data file not open\n");)
}


// --------------------------------------------------------------------------------------
void HttpRequest::CloseBody()
{
	if (m_body_file.get())
		m_body_file -> fclose();
DEB(	else
		fprintf(stderr, "CloseBody: File not open\n");)
}


// --------------------------------------------------------------------------------------
void HttpRequest::ParseBody()
{
	Utility::ncmap<std::string>::const_iterator it;
	if ( (it = m_attribute.find("query_string")) != m_attribute.end())
	{
		std::string qs = it -> second;
		m_form = std::auto_ptr<HttpdForm>(new HttpdForm( qs, qs.size() ));
	}
	else
	if (m_body_file.get())
	{
		m_form = std::auto_ptr<HttpdForm>(new HttpdForm( m_body_file.get(), ContentType(), ContentLength() ));
	}
	else
	{
		// dummy
		m_form = std::auto_ptr<HttpdForm>(new HttpdForm( "", 0 ));
	}
}


// --------------------------------------------------------------------------------------
const HttpdForm& HttpRequest::Form() const
{
	if (!m_form.get())
		throw Exception("Form not available");
	return *m_form;
}


// --------------------------------------------------------------------------------------
const HttpdCookies& HttpRequest::Cookies() const
{
	return m_cookies;
}


// --------------------------------------------------------------------------------------
void HttpRequest::Reset()
{
	HttpTransaction::Reset();
	m_method = "";
	m_protocol = "";
	m_req_uri = "";
	m_remote_addr = "";
	m_remote_host = "";
	m_server_name = "";
	m_server_port = 0;
	m_is_ssl = false;
	while (!m_attribute.empty())
	{
		m_attribute.erase(m_attribute.begin());
	}
	m_body_file = std::auto_ptr<IFile>(NULL);
	m_form = std::auto_ptr<HttpdForm>(NULL);
	m_cookies.Reset();
	while (!m_cookie.empty())
	{
		m_cookie.erase(m_cookie.begin());
	}
}


#ifdef SOCKETS_NAMESPACE
} // namespace SOCKETS_NAMESPACE {
#endif


