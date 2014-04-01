/** \file HttpPostSocket.cpp
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
#pragma warning(disable:4503)
#endif
#else
#include <errno.h>
#include <ctype.h>
#endif
#include "ISocketHandler.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "Lock.h"
#include "File.h"

#include "HttpPostSocket.h"


#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif


int HttpPostSocket::m_boundary_count = 0;
Mutex HttpPostSocket::m_boundary_mutex;


HttpPostSocket::HttpPostSocket(ISocketHandler& h) : HttpClientSocket(h)
,m_bMultipart(false)
{
}


HttpPostSocket::HttpPostSocket(ISocketHandler& h,const std::string& url_in) : HttpClientSocket(h, url_in)
,m_bMultipart(false)
{
	Lock lock(m_boundary_mutex);

	m_boundary = "----";
	for (int i = 0; i < 12; i++)
	{
		char c = m_boundary_count++ % 128;
		while (!isalnum(c))
			c = m_boundary_count++ % 128;
		m_boundary += c;
	}
	m_boundary += "__" + Utility::l2string(m_boundary_count++);
}


HttpPostSocket::~HttpPostSocket()
{
}


void HttpPostSocket::AddField(const std::string& name,const std::string& value)
{
	std::list<std::string> vec;
	vec.push_back(value);
	AddMultilineField(name, vec);
}


void HttpPostSocket::AddMultilineField(const std::string& name,std::list<std::string>& values)
{
	m_fields[name] = values;
}


void HttpPostSocket::AddFile(const std::string& name,const std::string& filename,const std::string& type)
{
	struct stat st;
	if (!stat(filename.c_str(), &st))
	{
		m_files[name] = filename;
		m_content_length[filename] = st.st_size;
		m_content_type[filename] = type;
		m_bMultipart = true;
	}
	else
	{
		Handler().LogError(this, "AddFile", Errno, StrError(Errno), LOG_LEVEL_FATAL);
		SetCloseAndDelete();
	}
}


void HttpPostSocket::Open()
{
	// why do I have to specify TcpSocket:: to get to the Open() method??
	TcpSocket::Open(GetUrlHost(), GetUrlPort());
}


void HttpPostSocket::OnConnect()
{
	if (m_bMultipart)
	{
		DoMultipartPost();
	}
	else
	{
		std::string body;

		// only fields, no files, add urlencoding
		for (std::map<std::string,std::list<std::string> >::iterator it = m_fields.begin(); it != m_fields.end(); ++it)
		{
			std::string name = (*it).first;
			std::list<std::string>& ref = (*it).second;
			if (body.size())
			{
				body += '&';
			}
			body += name + "=";
			bool first = true;
			for (std::list<std::string>::iterator it = ref.begin(); it != ref.end(); ++it)
			{
				std::string value = *it;
				if (!first)
				{
					body += "%0d%0a"; // CRLF
				}
				body += Utility::rfc1738_encode(value);
				first = false;
			}
		}

		// build header, send body
		SetMethod("POST");
		SetHttpVersion( "HTTP/1.1" );
		AddResponseHeader( "Host", GetUrlHost() ); // oops - this is actually a request header that we're adding..
		AddResponseHeader( "User-agent", MyUseragent());
		AddResponseHeader( "Accept", "text/html, text/plain, */*;q=0.01" );
		AddResponseHeader( "Connection", "close" );
		AddResponseHeader( "Content-type", "application/x-www-form-urlencoded" );
		AddResponseHeader( "Content-length", Utility::l2string((long)body.size()) );
		SendRequest();

		// send body
		Send( body );
	}
}


void HttpPostSocket::DoMultipartPost()
{
	long length = 0; // calculate content_length of our post body
	std::string tmp;

	// fields
	{
		for (std::map<std::string,std::list<std::string> >::iterator it = m_fields.begin(); it != m_fields.end(); ++it)
		{
			std::string name = (*it).first;
			std::list<std::string>& ref = (*it).second;
			tmp = "--" + m_boundary + "\r\n"
				"content-disposition: form-data; name=\"" + name + "\"\r\n"
				"\r\n";
			for (std::list<std::string>::iterator it = ref.begin(); it != ref.end(); ++it)
			{
				std::string value = *it;
				tmp += value + "\r\n";
			}
			length += (long)tmp.size();
		}
	}

	// files
	{
		for (std::map<std::string,std::string>::iterator it = m_files.begin(); it != m_files.end(); ++it)
		{
			std::string name = (*it).first;
			std::string filename = (*it).second;
			long content_length = m_content_length[filename];
			std::string content_type = m_content_type[filename];
			tmp = "--" + m_boundary + "\r\n"
				"content-disposition: form-data; name=\"" + name + "\"; filename=\"" + filename + "\"\r\n"
				"content-type: " + content_type + "\r\n"
				"\r\n";
			length += (long)tmp.size();
			length += content_length;
			length += 2; // crlf after file
		}
	}

	// end
	tmp = "--" + m_boundary + "--\r\n";
	length += (long)tmp.size();

	// build header, send body
	SetMethod("POST");
	SetHttpVersion( "HTTP/1.1" );
	AddResponseHeader( "Host", GetUrlHost() ); // oops - this is actually a request header that we're adding..
	AddResponseHeader( "User-agent", MyUseragent());
	AddResponseHeader( "Accept", "text/html, text/plain, */*;q=0.01" );
	AddResponseHeader( "Connection", "close" );
	AddResponseHeader( "Content-type", "multipart/form-data; boundary=" + m_boundary );
	AddResponseHeader( "Content-length", Utility::l2string(length) );

	SendRequest();

	// send fields
	{
		for (std::map<std::string,std::list<std::string> >::iterator it = m_fields.begin(); it != m_fields.end(); ++it)
		{
			std::string name = (*it).first;
			std::list<std::string>& ref = (*it).second;
			tmp = "--" + m_boundary + "\r\n"
				"content-disposition: form-data; name=\"" + name + "\"\r\n"
				"\r\n";
			for (std::list<std::string>::iterator it = ref.begin(); it != ref.end(); ++it)
			{
				std::string value = *it;
				tmp += value + "\r\n";
			}
			Send( tmp );
		}
	}

	// send files
	{
		for (std::map<std::string,std::string>::iterator it = m_files.begin(); it != m_files.end(); ++it)
		{
			std::string name = (*it).first;
			std::string filename = (*it).second;
			std::string content_type = m_content_type[filename];
			tmp = "--" + m_boundary + "\r\n"
				"content-disposition: form-data; name=\"" + name + "\"; filename=\"" + filename + "\"\r\n"
				"content-type: " + content_type + "\r\n"
				"\r\n";
			Send( tmp );
			{
				std::auto_ptr<IFile> fil = std::auto_ptr<IFile>(new File);
				if (fil -> fopen(filename, "rb"))
				{
					char slask[2000]; // for fread
					size_t n;
					while ((n = fil -> fread(slask, 1, 2000)) > 0)
					{
						SendBuf(slask, n);
					}
					fil -> fclose();
				}
			}
			Send("\r\n");
		}
	}

	// end of send
	Send("--" + m_boundary + "--\r\n");
}


void HttpPostSocket::SetMultipart()
{
	m_bMultipart = true;
}


#ifdef SOCKETS_NAMESPACE
}
#endif


