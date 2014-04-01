/** \file EventHandler.cpp
 **	\date  2005-12-07
 **	\author grymse@alhem.net
**/
/*
Copyright (C) 2005-2011  Anders Hedstrom

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
#include "EventHandler.h"
#include "IEventOwner.h"
#include "Event.h"
#include "TcpSocket.h"
#include "ListenSocket.h"


#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif


EventHandler::EventHandler(StdLog *p) : SocketHandler(p), m_quit(false)
{
	EnableRelease();
}


EventHandler::EventHandler(IMutex& m,StdLog *p) : SocketHandler(m, p), m_quit(false)
{
	EnableRelease();
}


EventHandler::~EventHandler()
{
	while (m_events.size())
	{
		std::list<Event *>::iterator it = m_events.begin();
		Event *e = *it;
		e -> GetFrom() -> SetHandlerInvalid();
		delete e;
		m_events.erase(it);
	}
}


bool EventHandler::GetTimeUntilNextEvent(struct timeval *tv)
{
	if (!m_events.size())
		return false;
	std::list<Event *>::iterator it = m_events.begin();
	if (it != m_events.end())
	{
		EventTime now;
		mytime_t diff = (*it) -> GetTime() - now;
		if (diff < 1)
		{
			diff = 1;
		}
		tv -> tv_sec = static_cast<long>(diff / 1000000);
		tv -> tv_usec = static_cast<long>(diff % 1000000);
		return true;
	}
	return false;
}


void EventHandler::CheckEvents()
{
	EventTime now;
	std::list<Event *>::iterator it = m_events.begin();
	while (it != m_events.end() && (*it) -> GetTime() < now)
	{
		Event *e = *it;
		Socket *s = dynamic_cast<Socket *>(e -> GetFrom());
		/*
		s == NULL    This is another object implementing 'IEventOwner' and not a socket.
		s != NULL    This is a Socket implementing IEventOwner, and we can check that the
			     object instance still is valid using SocketHandler::Valid.
		*/
		if (!s || (s && Valid( e -> Data() )))
		{
			e -> GetFrom() -> OnEvent(e -> GetID());
		}
		for (it = m_events.begin(); it != m_events.end(); ++it)
			if (*it == e)
			{
				delete e;
				break;
			}
		if (it != m_events.end())
			m_events.erase(it);
		it = m_events.begin();
	}
}


long EventHandler::AddEvent(IEventOwner *from,long sec,long usec)
{
	Socket *s = dynamic_cast<Socket *>(from);
	Event *e = new Event(from, sec, usec, s ? s -> UniqueIdentifier() : 0);
	std::list<Event *>::iterator it = m_events.begin();
	while (it != m_events.end() && *(*it) < *e)
	{
		++it;
	}
	m_events.insert(it, e);
	Release();
	return e -> GetID();
}


void EventHandler::ClearEvents(IEventOwner *from)
{
	bool repeat;
	do
	{
		repeat = false;
		for (std::list<Event *>::iterator it = m_events.begin(); it != m_events.end(); ++it)
		{
			Event *e = *it;
			if (e -> GetFrom() == from)
			{
				delete e;
				m_events.erase(it);
				repeat = true;
				break;
			}
		}
	} while (repeat);
}


void EventHandler::EventLoop()
{
	while (!m_quit)
	{
		struct timeval tv;
		if (GetTimeUntilNextEvent(&tv))
		{
			Select(&tv);
			CheckEvents();
		}
		else
		{
			Select();
		}
	}
}


void EventHandler::SetQuit(bool x)
{
	m_quit = x;
}


void EventHandler::RemoveEvent(IEventOwner *from, long eid)
{
	for (std::list<Event *>::iterator it = m_events.begin(); it != m_events.end(); ++it)
	{
		Event *e = *it;
		if (from == e -> GetFrom() && eid == e -> GetID())
		{
			delete e;
			m_events.erase(it);
			break;
		}
	}
}


void EventHandler::Add(Socket *p)
{
	SocketHandler::Add( p );
}


#ifdef SOCKETS_NAMESPACE
}
#endif



