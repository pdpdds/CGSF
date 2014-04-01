/**
 **	\file Ajp13Socket.cpp
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
#include "Ajp13Socket.h"
#include "ajp13.h"

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif

#ifdef _DEBUG
#define DEB(x) x
#else
#define DEB(x) 
#endif


// --------------------------------------------------------------------------------------
Ajp13Socket::Ajp13Socket(ISocketHandler& h) : AjpBaseSocket(h)
, m_body_size_left(0)
, m_b_reused(false)
{
}


// --------------------------------------------------------------------------------------
void Ajp13Socket::OnHeader( short id, short len )
{
	if (id != 0x1234)
	{
DEB(		fprintf(stderr, "ABORT: bad packet id: %x\n", id);)
		SetCloseAndDelete();
	}
	else
	{
		DEB(fprintf(stderr, "Packet size: %d bytes\n", len);)
	}
}


// --------------------------------------------------------------------------------------
void Ajp13Socket::ReceiveBody(const char *buf, size_t sz)
{
	if (sz - 2 > m_body_size_left)
	{
DEB(		fprintf(stderr, "More body data received than expected\n");)
		SetCloseAndDelete();
		return;
	}

	m_req.Write( buf + 2, sz - 2 );
	m_body_size_left -= sz - 2;

	// request more body data
	if (m_body_size_left)
	{
		int ptr = 4;
		char msg[100];
		msg[0] = 'A';
		msg[1] = 'B';

// reply codes
//	0x3 Send Body Chunk
//	0x4 Send Headers
//	0x5 End Response
//	0x6 Get Body Chunk	<------
//	0x9 CPong Reply

		put_byte(msg, ptr, 0x06); // GET_BODY_CHUNK;
		put_integer(msg, ptr, 1000); // request 1000 bytes

		short len = htons( ptr - 4 );
		memcpy( msg + 2, &len, 2 );

		SendBuf( msg, ptr );
		return;
	}

	// Close
	m_req.CloseBody();

	// no more body data left to read - execute
	Execute();

}


// --------------------------------------------------------------------------------------
void Ajp13Socket::ReceiveForwardRequest( const char *buf, size_t sz )
{
	//
	int ptr = 0;

	get_byte(buf, ptr); // skip first byte: prefix_code
	unsigned char method = get_byte(buf, ptr);
	std::string	 protocol = get_string(buf, ptr);
	std::string	 req_uri = get_string(buf, ptr);
	std::string	 remote_addr = get_string(buf, ptr);
	std::string	 remote_host = get_string(buf, ptr);
	std::string	 server_name = get_string(buf, ptr);
	short				 server_port = get_integer(buf, ptr);
	bool					is_ssl = get_boolean(buf, ptr);

	std::string method_str = Utility::l2string( method );
	std::map<int, std::string>::const_iterator it = Init.Method.find( method );
	if (it != Init.Method.end())
	{
		method_str = it -> second;
	}
	m_req.SetHttpMethod( method_str );
	m_req.SetHttpVersion( protocol );
	m_req.SetUri( req_uri );
	m_req.SetRemoteAddr( remote_addr );
	m_req.SetRemoteHost( remote_host );
	m_req.SetServerName( server_name );
	m_req.SetServerPort( server_port );
	m_req.SetIsSsl( is_ssl );

	// Get Headers
	short				 num_headers = get_integer(buf, ptr);
	for (int i = 0; i < num_headers; i++)
	{
		std::string key;
		if ( (unsigned char)buf[ptr] == 0xa0)
		{
			unsigned short x = (unsigned short)get_integer(buf, ptr);
			std::map<int, std::string>::const_iterator it;
			if ( (it = Init.Header.find(x)) != Init.Header.end())
			{
				key = it -> second;
			}
			else
			{
DEB(				fprintf(stderr, "Unknown header key value: %x\n", x);)
				SetCloseAndDelete();
				break; // don't attempt to parse more headers
			}
		}
		else
		{
			key = get_string(buf, ptr);
		}
		if (Utility::ToLower(key) == "cookie" || Utility::ToLower(key) == "cookie2")
			m_req.AddCookie(get_string(buf, ptr));
		else
			m_req.SetHeader(key, get_string(buf, ptr));
	} // for

	// size left to read from web server
	m_body_size_left = m_req.ContentLength();

	// Get Attributes
	bool ok = true;
	while ( (unsigned char)buf[ptr] != 0xff)
	{
		std::string key;
		unsigned char code = buf[ptr++];
		switch ( code)
		{
		case 10: // req_attribute, attribute name follow
			key = get_string(buf, ptr);
			break;
		default:
			{
				std::map<int, std::string>::const_iterator it = Init.Attribute.find( code );
				if (it != Init.Attribute.end())
				{
					key = it -> second;
				}
				else
				{
DEB(					fprintf(stderr, "Unknown attribute key: 0x%02x\n", buf[ptr]);)
					SetCloseAndDelete();
					ok = false;
				}
			}
		}
		if (!ok)
			break;
		if (code == 11)
			m_req.SetAttribute(key, get_integer(buf, ptr));
		else
			m_req.SetAttribute(key, get_string(buf, ptr));
	} // while

	// execute at once if no body data
	if (!m_body_size_left)
	{
		Execute();
	}
	else
	{
		// open temporary file for body data
		m_req.InitBody( m_body_size_left );
	}
}


// --------------------------------------------------------------------------------------
void Ajp13Socket::ReceiveShutdown( const char *buf, size_t sz )
{
}


// --------------------------------------------------------------------------------------
void Ajp13Socket::ReceivePing( const char *buf, size_t sz )
{
}


// --------------------------------------------------------------------------------------
void Ajp13Socket::ReceiveCPing( const char *buf, size_t sz )
{
       char msg[5]; 
       msg[0] = 'A';
       msg[1] = 'B';

       int ptr = 4;
       put_byte(msg, ptr, 0x09); // send CPong Reply

       short len = htons( ptr - 4 );
       memcpy( msg + 2, &len, 2 );

       SendBuf( msg, ptr );

       if (m_b_reused)
               reset();
}


// --------------------------------------------------------------------------------------
void Ajp13Socket::Execute()
{
	// parse form data / query_string and cookie header if available
	m_req.ParseBody();

	// prepare page
	IHttpServer_OnExec( m_req );

}


// --------------------------------------------------------------------------------------
void Ajp13Socket::IHttpServer_Respond(const HttpResponse& res)
{
	char msg[8192];
	msg[0] = 'A';
	msg[1] = 'B';

	m_res = res;

// reply codes
//	0x3 Send Body Chunk
//	0x4 Send Headers
//	0x5 End Response
//	0x6 Get Body Chunk
//	0x9 CPong Reply

	// check content length
	if (!m_res.ContentLength() && m_res.GetFile().size())
	{
//		m_res.SetContentLength( m_res.GetFile().size() );
	}

	// Send Headers
	{
		int ptr = 4;
		put_byte(msg, ptr, 0x04); // send headers
		put_integer(msg, ptr, m_res.HttpStatusCode() );
		put_string(msg, ptr, m_res.HttpStatusMsg() );
		put_integer(msg, ptr, (short)m_res.Headers().size() );
		for (Utility::ncmap<std::string>::const_iterator it = m_res.Headers().begin(); it != m_res.Headers().end(); ++it)
		{
			Utility::ncmap<int>::const_iterator it2 = Init.ResponseHeader.find( it -> first );
			if (it2 != Init.ResponseHeader.end())
			{
				put_integer(msg, ptr, it2 -> second);
			}
			else
			{
				put_string(msg, ptr, it -> first);
			}
			put_string(msg, ptr, it -> second);
		}
		std::list<std::string> vec = m_res.CookieNames();
		{
			for (std::list<std::string>::iterator it = vec.begin(); it != vec.end(); ++it)
			{
				Utility::ncmap<int>::const_iterator it2 = Init.ResponseHeader.find( "set-cookie" );
				if (it2 != Init.ResponseHeader.end())
				{
					put_integer(msg, ptr, it2 -> second);
				}
				else
				{
					put_string(msg, ptr, "set-cookie");
				}
				put_string(msg, ptr, m_res.Cookie(*it) );
			}
		}

		short len = htons( ptr - 4 );
		memcpy( msg + 2, &len, 2 );

		SendBuf( msg, ptr );
	}
	// Send Body Chunk
	OnTransferLimit();
}


// --------------------------------------------------------------------------------------
void Ajp13Socket::OnTransferLimit()
{
	char msg[8192];
	msg[0] = 'A';
	msg[1] = 'B';

	// Send Body Chunk
	size_t n = m_res.GetFile().fread(msg + 7, 1, 8100);
	while (n > 0)
	{
		int ptr = 4;
		put_byte(msg, ptr, 0x03); // send body chunk
		put_integer(msg, ptr, (short)n);
		ptr += (int)n;
		put_byte(msg, ptr, 0); // chunk terminator

		short len = htons( ptr - 4 );
		memcpy( msg + 2, &len, 2 );

		SendBuf( msg, ptr );
		if (GetOutputLength() > 1)
		{
			SetTransferLimit( 1 );
			break;
		}

		//
		n = m_res.GetFile().fread(msg + 7, 1, 8100);
	}
	if (!GetOutputLength()) // all body data sent and no data in output buffer - send end response
	{
		// End Response
		int ptr = 4;
		put_byte(msg, ptr, 0x05); // end response
		put_boolean(msg, ptr, m_b_reused); // reuse
		/*
			don't reuse
			- but with m_req.Reset() and m_res.Reset() it should be possible
			- also reset any AjpBaseSocket/Ajp13Socket specific states
		*/

		short len = htons( ptr - 4 );
		memcpy( msg + 2, &len, 2 );

		SendBuf( msg, ptr );

		SetTransferLimit(0);
		m_res.GetFile().fclose();
		IHttpServer_OnResponseComplete();
		if (m_b_reused)
		{
			Reset();
		}
	}
}


// --------------------------------------------------------------------------------------
void Ajp13Socket::Reset()
{
	reset();
	m_body_size_left = 0;
	m_req.Reset();
	m_res.Reset();
}


// --------------------------------------------------------------------------------------
void Ajp13Socket::OnPacket( const char *buf, size_t sz )
{
	DEB(fprintf(stderr, "OnPacket: %d bytes, code 0x%02x %02x %02x %02x\n", sz, *buf, buf[1], buf[2], buf[3]);)

	// check body size left to read, if non-zero packet is body data
	if (m_body_size_left) // must be a body packet
	{
		ReceiveBody(buf, sz);
		return;
	}
	switch (*buf)
	{
	case 0x2: // Forward Request
		ReceiveForwardRequest(buf, sz);
		break;
	case 0x7: // Shutdown
		ReceiveShutdown(buf, sz);
		break;
	case 0x8: // Ping
		ReceivePing(buf, sz);
		break;
	case 0xa: // CPing
		ReceiveCPing(buf, sz);
		break;
	default:
DEB(		fprintf(stderr, "Unknown packet type: 0x%02x\n", *buf);)
		SetCloseAndDelete();
	}

}


// --------------------------------------------------------------------------------------
void Ajp13Socket::SetReused(bool x)
{
	m_b_reused = x;
}


// --------------------------------------------------------------------------------------
bool Ajp13Socket::IsReused()
{
	return m_b_reused;
}


#ifdef SOCKETS_NAMESPACE
} // namespace SOCKETS_NAMESPACE {
#endif


