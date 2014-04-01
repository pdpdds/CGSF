/** \file HttpdCookies.cpp
*/
/*
Copyright (C) 2003-2011  Anders Hedstrom

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
#include "Parse.h"
#include "HTTPSocket.h"
#include "HttpdCookies.h"
#include <cstdio>

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif

#ifdef _DEBUG
#define DEB(x) x; fflush(stderr);
#else
#define DEB(x)
#endif


HttpdCookies::HttpdCookies()
{
}

HttpdCookies::HttpdCookies(const std::string& s)
{
	Parse *pa = new Parse(s,";");

	std::string slask = pa -> getword();
	while (slask.size())
	{
		Parse *pa2 = new Parse(slask,"=");
		std::string name = pa2 -> getword();
		std::string value = pa2 -> getword();
		delete pa2;
		m_cookies.push_back(std::pair<std::string, std::string>(name, value));
		//
		slask = pa -> getword();
	}
	delete pa;
}

void HttpdCookies::add(const std::string& s)
{
	Parse *pa = new Parse(s,";");
DEB(fprintf(stderr, "Parse cookie: %s\n", s.c_str());)
	std::string slask = pa -> getword();
	while (slask.size())
	{
		Parse *pa2 = new Parse(slask,"=");
		std::string name = pa2 -> getword();
		std::string value = pa2 -> getword();
		delete pa2;
		m_cookies.push_back(std::pair<std::string, std::string>(name, value));
		//
		slask = pa -> getword();
	}
	delete pa;
}

HttpdCookies::~HttpdCookies()
{
}

bool HttpdCookies::getvalue(const std::string& name,std::string& buffer) const
{
	for (cookie_v::const_iterator it = m_cookies.begin(); it != m_cookies.end(); ++it)
	{
		const std::pair<std::string, std::string>& ref = *it;
		if (!strcasecmp(ref.first.c_str(),name.c_str()))
		{
			buffer = ref.second;
			return true;
		}
	}
	buffer = "";
	return false;
}

void HttpdCookies::replacevalue(const std::string& name,const std::string& value)
{
	for (cookie_v::iterator it = m_cookies.begin(); it != m_cookies.end(); ++it)
	{
		std::pair<std::string, std::string>& ref = *it;
		if (!strcasecmp(ref.first.c_str(),name.c_str()))
		{
			ref.second = value;
			return;
		}
	}
	m_cookies.push_back(std::pair<std::string, std::string>(name, value));

}

void HttpdCookies::replacevalue(const std::string& name,long l)
{
	replacevalue(name, Utility::l2string(l));
}

void HttpdCookies::replacevalue(const std::string& name,int i)
{
	replacevalue(name, Utility::l2string(i));
}

size_t HttpdCookies::getlength(const std::string& name) const
{
	for (cookie_v::const_iterator it = m_cookies.begin(); it != m_cookies.end(); ++it)
	{
		const std::pair<std::string, std::string>& ref = *it;
		if (!strcasecmp(ref.first.c_str(),name.c_str()))
		{
			return ref.second.size();
		}
	}
	return 0;
}

void HttpdCookies::setcookie(HTTPSocket *sock, const std::string& domain, const std::string& path, const std::string& name, const std::string& value)
{
	size_t sz = name.size() + value.size() + domain.size() + path.size() + 100;
	char *str = new char[sz];

	// set-cookie response
	if (domain.size())
	{
		snprintf(str, sz, "%s=%s; domain=%s; path=%s; expires=%s",
		 name.c_str(), value.c_str(),
		 domain.c_str(),
		 path.c_str(),
		 expiredatetime().c_str());
	}
	else
	{
		snprintf(str, sz, "%s=%s; path=%s; expires=%s",
		 name.c_str(), value.c_str(),
		 path.c_str(),
		 expiredatetime().c_str());
	}
	sock -> AddResponseHeader("Set-cookie", str);
	delete[] str;

	replacevalue(name, value);
}

void HttpdCookies::setcookie(HTTPSocket *sock, const std::string& domain, const std::string& path, const std::string& name, long value)
{
	size_t sz = name.size() + domain.size() + path.size() + 100;
	char *str = new char[sz];
	char dt[80];

	// set-cookie response
	if (domain.size())
	{
		snprintf(str, sz, "%s=%ld; domain=%s; path=%s; expires=%s",
		 name.c_str(), value,
		 domain.c_str(),
		 path.c_str(),
		 expiredatetime().c_str());
	}
	else
	{
		snprintf(str, sz, "%s=%ld; path=%s; expires=%s",
		 name.c_str(), value,
		 path.c_str(),
		 expiredatetime().c_str());
	}
	sock -> AddResponseHeader("Set-cookie", str);
	delete[] str;

	snprintf(dt, sizeof(dt), "%ld", value);
	replacevalue(name, dt);
}

void HttpdCookies::setcookie(HTTPSocket *sock, const std::string& domain, const std::string& path, const std::string& name, int value)
{
	size_t sz = name.size() + domain.size() + path.size() + 100;
	char *str = new char[sz];
	char dt[80];

	// set-cookie response
	if (domain.size())
	{
		snprintf(str, sz, "%s=%d; domain=%s; path=%s; expires=%s",
		 name.c_str(), value,
		 domain.c_str(),
		 path.c_str(),
		 expiredatetime().c_str());
	}
	else
	{
		snprintf(str, sz, "%s=%d; path=%s; expires=%s",
		 name.c_str(), value,
		 path.c_str(),
		 expiredatetime().c_str());
	}
	sock -> AddResponseHeader("Set-cookie", str);
	delete[] str;

	snprintf(dt, sizeof(dt), "%d", value);
	replacevalue(name, dt);
}


const std::string& HttpdCookies::expiredatetime() const
{
	const char *days[7] = {"Sunday", "Monday",
	 "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
	const char *months[12] = {"Jan", "Feb", "Mar", "Apr", "May",
	 "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
	char dt[100];

	time_t t = time(NULL);
#ifdef __CYGWIN__
	struct tm *tp = gmtime(&t);
	snprintf(dt, sizeof(dt), "%s, %02d-%s-%04d %02d:%02d:%02d GMT",
	 days[tp -> tm_wday],
	 tp -> tm_mday,
	 months[tp -> tm_mon],
	 tp -> tm_year + 1910,
	 tp -> tm_hour,
	 tp -> tm_min,
	 tp -> tm_sec);
#else
	struct tm tp;
#if defined( _WIN32) && !defined(__CYGWIN__)
	gmtime_s(&tp, &t);
#else
	gmtime_r(&t, &tp);
#endif
	snprintf(dt, sizeof(dt), "%s, %02d-%s-%04d %02d:%02d:%02d GMT",
	 days[tp.tm_wday],
	 tp.tm_mday,
	 months[tp.tm_mon],
	 tp.tm_year + 1910,
	 tp.tm_hour,
	 tp.tm_min,
	 tp.tm_sec);
#endif
	m_date = dt;
	return m_date;
}


void HttpdCookies::Reset()
{
	while (!m_cookies.empty())
	{
		m_cookies.erase(m_cookies.begin());
	}
	m_date = "";
}


#ifdef SOCKETS_NAMESPACE
}
#endif


