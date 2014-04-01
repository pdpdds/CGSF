/**
 **	\file SocketThread.h
 **	\date  2011-08-16
 **	\author grymse@alhem.net
**/
/*
Copyright (C) 2011  Anders Hedstrom

This library is made available under the terms of the GNU GPL.

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
#ifndef _SOCKETTHREAD_H
#define _SOCKETTHREAD_H

#include "sockets-config.h"
#ifdef ENABLE_DETACH

#include "Thread.h"
#include "SocketHandler.h"

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif

class Socket;

/** Detached socket run thread. 
	\ingroup internal */
class SocketThread : public Thread
{
public:
	SocketThread(Socket *p);
	~SocketThread();

	void Run();

private:
	SocketThread(const SocketThread& s) : m_socket(s.m_socket) {}
	SocketThread& operator=(const SocketThread& ) { return *this; }

	SocketHandler m_h;
	Socket *m_socket;
};

#ifdef SOCKETS_NAMESPACE
} // namespace SOCKETS_NAMESPACE {
#endif

#endif // ENABLE_DETACH

#endif // _SOCKETTHREAD_H
