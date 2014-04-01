/**
 **	\file Debug.h
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
#ifndef _SOCKETS_Debug_H
#define _SOCKETS_Debug_H

#include "sockets-config.h"
#include "Utility.h"
#include <stdio.h>


#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif


class Debug
{
static	const char *colors[];
public:
	class endl {
	public:
		endl() {}
		virtual ~endl() {}
	};

public:
	Debug() {}
	Debug(const std::string& x) : m_id(0), m_text(x) {
		fprintf(stderr, "%s", colors[Utility::ThreadID() % 14 + 1]);
		for (int i = 0; i < m_level[Utility::ThreadID()]; i++)
			fprintf(stderr, "  ");
		fprintf(stderr, "%s%s\n", x.c_str(), colors[0]);
		m_level[Utility::ThreadID()]++;
	}
	Debug(int id, const std::string& x) : m_id(id), m_text(x) {
		fprintf(stderr, "%s", colors[Utility::ThreadID() % 14 + 1]);
		for (int i = 0; i < m_level[Utility::ThreadID()]; i++)
			fprintf(stderr, "  ");
		fprintf(stderr, "%d> %s%s\n", m_id, x.c_str(), colors[0]);
		m_level[Utility::ThreadID()]++;
	}
	~Debug() {
		if (!m_text.empty())
		{
			if (m_level[Utility::ThreadID()])
				m_level[Utility::ThreadID()]--;
			fprintf(stderr, "%s", colors[Utility::ThreadID() % 14 + 1]);
			for (int i = 0; i < m_level[Utility::ThreadID()]; i++)
				fprintf(stderr, "  ");
			if (m_id)
				fprintf(stderr, "%d> /%s%s\n", m_id, m_text.c_str(), colors[0]);
			else
				fprintf(stderr, "/%s%s\n", m_text.c_str(), colors[0]);
			fflush(stderr);
		}
	}
static	void Print(const char *format, ...);

	Debug& operator<<(const char *);
	Debug& operator<<(const std::string& );
	Debug& operator<<(short);
	Debug& operator<<(int);
	Debug& operator<<(long);
	Debug& operator<<(double);
	Debug& operator<<(endl);

private:
	int m_id;
	std::string m_text;
static	std::map<unsigned long, int> m_level;
	std::string m_line;
};


#ifdef SOCKETS_NAMESPACE
} // namespace SOCKETS_NAMESPACE {
#endif

#endif // _SOCKETS_Debug_H

