/**
 **	\file SmtpdSocket.cpp
 **	\date  2007-05-10
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
#include "SmtpdSocket.h"
#include "Parse.h"
#include "Utility.h"

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif

SmtpdSocket::SmtpdSocket(ISocketHandler& h)
:TcpSocket(h)
,m_hello(false)
,m_data(false)
,m_header(false)
{
	SetLineProtocol();
}


void SmtpdSocket::OnAccept()
{
	Send("220 ESMTP; \r\n");
}


void SmtpdSocket::OnLine(const std::string& line)
{
	if (m_data)
	{
		if (m_header)
		{
			if (!line.size())
			{
				if (m_header_line.size())
				{
					Parse pa(m_header_line, ":");
					std::string key = pa.getword();
					OnHeader(key, pa.getrest());
				}
				m_header = false;
				OnHeaderComplete();
			}
			else
			if (line[0] == ' ' || line[0] == '\t')
			{
				m_header_line += line;
			}
			else
			{
				if (m_header_line.size())
				{
					Parse pa(m_header_line, ":");
					std::string key = pa.getword();
					OnHeader(key, pa.getrest());
				}
				m_header_line = line;
			}
		}
		else
		if (line == ".")
		{
			m_data = false;
			if (OnDataComplete())
				Send("250 OK\r\n");
			else
				Send("550 Failed\r\n");
		}
		else
		if (line.size() && line[0] == '.')
		{
			OnData(line.substr(1));
		}
		else
		{
			OnData(line);
		}
		return;
	}
	Parse pa(line);
	std::string cmd = Utility::ToUpper(pa.getword());

	if (cmd == "EHLO")
	{
		if (!OnHello(pa.getrest()))
		{
			Send("550 Failed\r\n");
		}
		else
		{
			m_hello = true;
			Send("250 mail.alhem.net\r\n");
		}
	}
	else
	if (cmd == "HELO")
	{
		if (!OnHello(pa.getrest()))
		{
			Send("550 Failed\r\n");
		}
		else
		{
			m_hello = true;
			Send("250 mail.alhem.net\r\n");
		}
	}
	else
	if (!m_hello)
	{
		OnAbort(SMTP_NO_HELLO);
		SetCloseAndDelete();
	}
	else
	if (cmd == "MAIL") // mail from:
	{
		Parse pa(line, ":");
		pa.getword(); // 'mail'
		pa.getword(); // 'from'
		std::string email = Utility::ToLower(pa.getrest());

		EmailAddress addr( email );
		if (addr.GetName().size() > 64)
		{
			OnAbort(SMTP_NAME_TOO_LONG);
			Send("500 Name too long.\r\n");
			return;
		}
		if (addr.GetDomain().size() > 64)
		{
			OnAbort(SMTP_DOMAIN_TOO_LONG);
			Send("500 Domain too long.\r\n");
			return;
		}

		if (!OnMailFrom( addr ))
		{
			Send("550 Failed\r\n");
		}
		else
		{
			Send("250 OK\r\n");
		}
	}
	else
	if (cmd == "RCPT") // rcpt to:
	{
		Parse pa(line, ":");
		pa.getword(); // 'rcpt'
		pa.getword(); // 'to'
		std::string email = Utility::ToLower(pa.getrest());
		// %! reject based on user / domain?
		EmailAddress addr( email );

		if (addr.GetName().size() > 64)
		{
			OnAbort(SMTP_NAME_TOO_LONG);
			Send("500 Name too long.\r\n");
			return;
		}
		if (addr.GetDomain().size() > 64)
		{
			OnAbort(SMTP_DOMAIN_TOO_LONG);
			Send("500 Domain too long.\r\n");
			return;
		}

		if (!OnRcptTo( addr ))
		{
			Send("553 Failed\r\n");
		}
		else
		{
			Send("250 OK\r\n");
		}
	}
	else
	if (cmd == "DATA")
	{
		Send("354 Enter mail, end with \".\" on a line by itself\r\n");
		m_data = true;
		m_header = false;
	}
	else
	if (cmd == "RSET")
	{
		m_data = false;
		m_header = false;
		OnRset();
		Send("250 OK\r\n"); // %! ???
	}
	else
	if (cmd == "QUIT")
	{
		OnAbort(SMTP_QUIT);
		Send("221 Bye Bye\r\n");
		SetCloseAndDelete();
	}
	else
	if (cmd == "NOOP")
	{
		Send("250 OK\r\n");
	}
	else
	{
		OnNotSupported(cmd, pa.getrest());
	}
}


#ifdef SOCKETS_NAMESPACE
} // namespace SOCKETS_NAMESPACE {
#endif

