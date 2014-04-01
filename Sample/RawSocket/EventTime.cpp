/** \file EventTime.cpp
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
#ifdef MACOSX
#include <stdint.h>
#include <sys/types.h>
#include <signal.h>
#endif
#include "EventTime.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/select.h>
#include <sys/time.h>
#endif



#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif


EventTime::EventTime() : m_time(Tick())
{
}


EventTime::EventTime(mytime_t sec,long usec) : m_time(Tick())
{
	m_time += sec * 1000000 + usec;
}


EventTime::~EventTime()
{
}


mytime_t EventTime::Tick()
{
	mytime_t t;
#ifdef _WIN32
	FILETIME ft;
	GetSystemTimeAsFileTime(&ft);
	t = ft.dwHighDateTime;
	t = t << 32;
	t += ft.dwLowDateTime;
	t /= 10; // us
#else
	struct timeval tv;
	struct timezone tz;
	gettimeofday(&tv, &tz);
	t = tv.tv_sec;
	t *= 1000000;
	t += tv.tv_usec;
#endif
	return t;
}


EventTime EventTime::operator - (const EventTime& x) const
{
	EventTime t;
	t.m_time = m_time - x.m_time;
	return t;
}


bool EventTime::operator < (const EventTime& x) const
{
	return m_time < x.m_time;
}


#ifdef SOCKETS_NAMESPACE
}
#endif

