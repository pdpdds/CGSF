/** \file HttpPutSocket.cpp
 **	\date  2004-10-30
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
#ifdef _WIN32
#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif
#else
#include <errno.h>
#endif
#include "ISocketHandler.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "File.h"

#include "HttpPutSocket.h"


#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif


HttpPutSocket::HttpPutSocket(ISocketHandler& h) : HttpClientSocket(h)
{
}


HttpPutSocket::HttpPutSocket(ISocketHandler& h,const std::string& url_in) : HttpClientSocket(h, url_in)
{
}


HttpPutSocket::~HttpPutSocket()
{
}


void HttpPutSocket::SetFile(const std::string& file)
{
	struct stat st;
	if (!stat(file.c_str(), &st))
	{
		m_filename = file;
		m_content_length = st.st_size;
	}
	else
	{
		Handler().LogError(this, "SetFile", Errno, StrError(Errno), LOG_LEVEL_FATAL);
		SetCloseAndDelete();
	}
}


void HttpPutSocket::SetContentType(const std::string& type)
{
	m_content_type = type;
}



void HttpPutSocket::Open()
{
	// why do I have to specify TcpSocket:: to get to the Open() method??
	TcpSocket::Open(GetUrlHost(), GetUrlPort());
}


void HttpPutSocket::OnConnect()
{
	SetMethod( "PUT" );
	SetHttpVersion( "HTTP/1.1" );
	AddResponseHeader( "Host", GetUrlHost() );
	AddResponseHeader( "Content-type", m_content_type );
	AddResponseHeader( "Content-length", Utility::l2string(m_content_length) );
	AddResponseHeader( "User-agent", MyUseragent() );
	SendRequest();

	std::auto_ptr<IFile> fil = std::auto_ptr<IFile>(new File);
	if (fil -> fopen(m_filename, "rb"))
	{
		size_t n;
		char buf[32768];
		while ((n = fil -> fread(buf, 1, 32768)) > 0)
		{
			SendBuf(buf, n);
		}
		fil -> fclose();
	}
}


#ifdef SOCKETS_NAMESPACE
}
#endif


