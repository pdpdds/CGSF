/**
 **	\file SocketThread.cpp
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
#include "SocketThread.h"

#ifdef ENABLE_DETACH
#include "Utility.h"

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif

SocketThread::SocketThread(Socket *p)
:Thread(false)
,m_socket(p)
{
	// Creator will release
}


SocketThread::~SocketThread()
{
	if (IsRunning())
	{
		SetRelease(true);
		SetRunning(false);
		m_h.Release();
		Utility::Sleep(5);
	}
}


void SocketThread::Run()
{
	m_h.SetSlave();
	m_h.Add(m_socket);
	m_socket -> SetSlaveHandler(&m_h);
	m_socket -> OnDetached();
	m_h.EnableRelease();
	while (m_h.GetCount() > 1 && IsRunning())
	{
		m_h.Select(0, 500000);
	}
	// m_socket now deleted oops
	//  (a socket can only be detached if DeleteByHandler() is true)
	// yeah oops m_socket delete its socket thread, that means this
	// so Socket will no longer delete its socket thread, instead we do this:
	SetDeleteOnExit();
}

#ifdef SOCKETS_NAMESPACE
} // namespace SOCKETS_NAMESPACE {
#endif

#endif // ENABLE_DETACH
