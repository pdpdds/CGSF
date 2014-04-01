/** \file HttpDebugSocket.cpp
 **	\date  2004-10-08
**/
/*
Copyright (C) 2004-2011  Anders Hedstrom (grymse@alhem.net)

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
#include "HttpDebugSocket.h"
#include "ISocketHandler.h"


#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif


HttpDebugSocket::HttpDebugSocket(ISocketHandler& h) : HTTPSocket(h)
,m_content_length(0)
,m_read_ptr(0)
{
}


HttpDebugSocket::~HttpDebugSocket()
{
}


void HttpDebugSocket::Init()
{
	if (GetParent() -> GetPort() == 443)
	{
#ifdef HAVE_OPENSSL
		EnableSSL();
#else
		Handler().LogError(this, "url_this", -1, "SSL not available", LOG_LEVEL_WARNING);
#endif
	}
}


void HttpDebugSocket::OnFirst()
{
	Send(
		"HTTP/1.1 200 OK\n"
		"Content-type: text/html\n"
		"Connection: close\n"
		"Server: HttpDebugSocket/1.0\n"
		"\n");
	Send(
		"<html><head><title>Echo Request</title></head>"
		"<body><h3>Request Header</h3>");
	Send(	"<form method='post' action='/test_post'>"
		"<input type='text' name='text' value='test text'><br>"
		"<input type='submit' name='submit' value=' OK '></form>");

	// enctype 'multipart/form-data'
	Sendf("<form action='/test_post' method='post' enctype='multipart/form-data'>");
	Sendf("<input type=file name=the_file><br>");
	Sendf("<input type=text name=the_name><br>");
	Sendf("<input type=submit name=submit value=' test form-data '>");
	Sendf("</form>");

	Send(	"<pre style='background: #e0e0e0'>");
	Send(GetMethod() + " " + GetUrl() + " " + GetHttpVersion() + "\n");
}


void HttpDebugSocket::OnHeader(const std::string& key,const std::string& value)
{
	if (!strcasecmp(key.c_str(),"content-length"))
		m_content_length = atoi(value.c_str());

	Send(key + ": " + value + "\n");
}


void HttpDebugSocket::OnHeaderComplete()
{
	if (m_content_length || IsChunked())
	{
		Send("</pre><h3>Request Body</h3><pre style='background: #e0e0e0'>");
	}
	else
	{
		Send("</pre><hr></body></html>");
		SetCloseAndDelete();
	}
}


void HttpDebugSocket::OnData(const char *p,size_t l)
{
	SendBuf(p,l);
	m_read_ptr += (int)l;
	if (m_read_ptr >= m_content_length && m_content_length)
	{
		Send("</pre><hr></body></html>");
		SetCloseAndDelete();
	}
}


void HttpDebugSocket::OnDataComplete()
{
	if (!CloseAndDelete())
	{
		Send("</pre><hr></body></html>");
		SetCloseAndDelete();
	}
}


#ifdef SOCKETS_NAMESPACE
}
#endif


