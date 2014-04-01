/**
 **	\file AjpBaseSocket.cpp
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
#include "AjpBaseSocket.h"
#include "ajp13.h"

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif

#ifdef _DEBUG
#define DEB(x) x
#else
#define DEB(x) 
#endif


// ---------------------------------------------------------------------------
AjpBaseSocket::Initializer AjpBaseSocket::Init;


// ---------------------------------------------------------------------------
AjpBaseSocket::Initializer::Initializer()
{

	Header[HTTP_REQUEST_ACCEPT] = "accept";
	Header[HTTP_REQUEST_ACCEPT_CHARSET] = "accept-charset";
	Header[HTTP_REQUEST_ACCEPT_ENCODING] = "accept-encoding";
	Header[HTTP_REQUEST_ACCEPT_LANGUAGE] = "accept-language";
	Header[HTTP_REQUEST_AUTHORIZATION] = "authorization";
	Header[HTTP_REQUEST_CONNECTION] = "connection";
	Header[HTTP_REQUEST_CONTENT_TYPE] = "content-type";
	Header[HTTP_REQUEST_CONTENT_LENGTH] = "content-length";
	Header[HTTP_REQUEST_COOKIE] = "cookie";
	Header[HTTP_REQUEST_COOKIE2] = "cookie2";
	Header[HTTP_REQUEST_HOST] = "host";
	Header[HTTP_REQUEST_PRAGMA] = "pragma";
	Header[HTTP_REQUEST_REFERER] = "referer";
	Header[HTTP_REQUEST_USER_AGENT] = "user-agent";

	Method[HTTP_METHOD_OPTIONS] = "OPTIONS";
	Method[HTTP_METHOD_GET] = "GET";
	Method[HTTP_METHOD_HEAD] = "HEAD";
	Method[HTTP_METHOD_POST] = "POST";
	Method[HTTP_METHOD_PUT] = "PUT";
	Method[HTTP_METHOD_DELETE] = "DELETE";
	Method[HTTP_METHOD_TRACE] = "TRACE";
	Method[HTTP_METHOD_PROPFIND] = "PROPFIND";
	Method[HTTP_METHOD_PROPPATCH] = "PROPPATCH";
	Method[HTTP_METHOD_MKCOL] = "MKCOL";
	Method[HTTP_METHOD_COPY] = "COPY";
	Method[HTTP_METHOD_MOVE] = "MOVE";
	Method[HTTP_METHOD_LOCK] = "LOCK";
	Method[HTTP_METHOD_UNLOCK] = "UNLOCK";
	Method[HTTP_METHOD_ACL] = "ACL";
	Method[HTTP_METHOD_REPORT] = "REPORT";
	Method[HTTP_METHOD_VERSION_CONTROL] = "VERSION_CONTROL"; // with a dash "VERSION-CONTROL"
	Method[HTTP_METHOD_CHECKIN] = "CHECKIN";
	Method[HTTP_METHOD_CHECKOUT] = "CHECKOUT";
	Method[HTTP_METHOD_UNCHECKOUT] = "UNCHECKOUT";
	Method[HTTP_METHOD_SEARCH] = "SEARCH";
	Method[HTTP_METHOD_MKWORKSPACE] = "MKWORKSPACE";
	Method[HTTP_METHOD_UPDATE] = "UPDATE";
	Method[HTTP_METHOD_LABEL] = "LABEL";
	Method[HTTP_METHOD_MERGE] = "MERGE";
	Method[HTTP_METHOD_BASELINE_CONTROL] = "BASELINE_CONTROL";
	Method[HTTP_METHOD_MKACTIVITY] = "MKACTIVITY";

	Attribute[ATTR_CONTEXT] = "context";
	Attribute[ATTR_SERVLET_PATH] = "servlet_path";
	Attribute[ATTR_REMOTE_USER] = "remote_user";
	Attribute[ATTR_AUTH_TYPE] = "auth_type";
	Attribute[ATTR_QUERY_STRING] = "query_string";
	Attribute[ATTR_ROUTE] = "route";
	Attribute[ATTR_SSL_CERT] = "ssl_cert";
	Attribute[ATTR_SSL_CIPHER] = "ssl_cipher";
	Attribute[ATTR_SSL_SESSION] = "ssl_session";
	Attribute[ATTR_SSL_KEY_SIZE] = "ssl_key_size";
	Attribute[ATTR_SECRET] = "secret";
	Attribute[ATTR_STORED_METHOD] = "stored_method";

	ResponseHeader["content-type"] = 0xa001;
	ResponseHeader["content-language"] = 0xa002;
	ResponseHeader["content-length"] = 0xa003;
	ResponseHeader["date"] = 0xa004;
	ResponseHeader["last-modified"] = 0xa005;
	ResponseHeader["location"] = 0xa006;
	ResponseHeader["set-cookie"] = 0xa007;
	ResponseHeader["set-cookie2"] = 0xa008;
	ResponseHeader["servlet-engine"] = 0xa009;
	ResponseHeader["status"] = 0xa00a;
	ResponseHeader["www-authenticate"] = 0xa00b;
}


// ---------------------------------------------------------------------------
AjpBaseSocket::AjpBaseSocket(ISocketHandler& h) : TcpSocket(h)
, m_state(0)
, m_length(4)
, m_ptr(0)
{
}


// ---------------------------------------------------------------------------
void AjpBaseSocket::OnRawData(const char *buf, size_t sz)
{
DEB(fprintf(stderr, "OnRawData: %d bytes\n", sz);)
	size_t ptr = 0;
	while (true)
	{
		size_t left = sz - ptr;
DEB(fprintf(stderr, " left: %d bytes\n", left);
fprintf(stderr, " state: %d\n", m_state);)
		switch (m_state)
		{
		case 0:
			{
				size_t missing = m_length - m_ptr;
				short len = (short)(missing < left ? missing : left);
				memcpy(m_message + m_ptr, buf + ptr, len);
				m_ptr += len;
				ptr += len;
				if (m_ptr < m_length)
				{
					return; // read more
				}
				int p = 0;
				short id = get_integer(m_message, p);
				short length = get_integer(m_message, p);
				OnHeader(id, length);
				m_state = 1;
				m_length = length;
				m_ptr = 0; // bytes in m_message
			}
			break;
		case 1:
			{
				size_t missing = m_length - m_ptr;
				short len = (short)(missing < left ? missing : left);
				memcpy(m_message + m_ptr, buf + ptr, len);
				m_ptr += len;
				ptr += len;
				if (m_ptr < m_length)
				{
					return; // read more
				}
				OnPacket(m_message, m_ptr);
				m_state = 0;
				m_length = 4;
				m_ptr = 0;
			}
			break;
		}
	}
}


// ---------------------------------------------------------------------------
unsigned char AjpBaseSocket::get_byte(const char *buf, int& ptr)
{
	return (unsigned char)buf[ptr++];
}


// ---------------------------------------------------------------------------
bool AjpBaseSocket::get_boolean(const char *buf, int& ptr)
{
	return ( (unsigned char)buf[ptr++] & 1) == 1 ? true : false;
}


// ---------------------------------------------------------------------------
short AjpBaseSocket::get_integer(const char *buf, int& ptr)
{
	short n;
	memcpy(&n, buf + ptr, 2);
	ptr += 2;
	return ntohs(n);
}


// ---------------------------------------------------------------------------
std::string AjpBaseSocket::get_string(const char *buf, int& ptr)
{
	short len = get_integer(buf, ptr);
	if (len != -1)
	{
		std::string tmp = buf + ptr;
		ptr += len;
		ptr++; // skip trailing 0x0
		tmp.resize(len);
		return tmp;
	}
	return "";
}


// ---------------------------------------------------------------------------
void AjpBaseSocket::put_byte(char *buf, int& ptr, unsigned char zz)
{
	buf[ptr++] = zz;
}


// ---------------------------------------------------------------------------
void AjpBaseSocket::put_boolean(char *buf, int& ptr, bool zz)
{
	buf[ptr++] = zz ? 1 : 0;
}


// ---------------------------------------------------------------------------
void AjpBaseSocket::put_integer(char *buf, int& ptr, short zz)
{
	short tmp = htons(zz);
	memcpy(buf + ptr, &tmp, 2);
	ptr += 2;
}


// ---------------------------------------------------------------------------
void AjpBaseSocket::put_string(char *buf, int& ptr, const std::string& zz)
{
	put_integer(buf, ptr, (short)zz.size() );
	memcpy(buf + ptr, zz.c_str(), zz.size());
	ptr += (int)zz.size();
	put_byte(buf, ptr, 0);
}


// ---------------------------------------------------------------------------
void AjpBaseSocket::reset()
{
	m_state = m_ptr = 0;
	m_length = 4;
}


#ifdef SOCKETS_NAMESPACE
} // namespace SOCKETS_NAMESPACE {
#endif


