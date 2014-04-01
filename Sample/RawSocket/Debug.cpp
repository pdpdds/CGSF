/**
 **	\file Debug.cpp
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
#include "Debug.h"
#include <stdarg.h>
#include <cstring>


#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif


std::map<unsigned long, int> Debug::m_level;
const char *Debug::colors[] = {
		"\x1B[0;0m", // &n */
		"\x1B[0;0m\x1B[31m", // &r */
		"\x1B[0;0m\x1B[32m", // &g */
		"\x1B[0;0m\x1B[33m", // &y */
		"\x1B[0;0m\x1B[34m", // &b */
		"\x1B[0;0m\x1B[35m", // &m */
		"\x1B[0;0m\x1B[36m", // &c */
		"\x1B[0;0m\x1B[37m", // &w */
		"\x1B[1;31m", // &R */
		"\x1B[1;32m", // &G */
		"\x1B[1;33m", // &Y */
		"\x1B[1;34m", // &B */
		"\x1B[1;35m", // &M */
		"\x1B[1;36m", // &C */
		"\x1B[1;37m" }; // &W */


void Debug::Print(const char *format, ...)
{
	char slask[5000]; // temporary for vsprintf / vsnprintf
	va_list ap;

	va_start(ap, format);
	vsnprintf(slask, sizeof(slask), format, ap);
	va_end(ap);

	fprintf(stderr, "%s", colors[Utility::ThreadID() % 14 + 1]);
	for (int i = 0; i < m_level[Utility::ThreadID()]; i++)
		fprintf(stderr, "  ");
	if (slask[strlen(slask) - 1] == '\n')
		slask[strlen(slask) - 1] = 0;
	fprintf(stderr, "%s%s\n", slask, colors[0]);
}


Debug& Debug::operator<<(const char * str)
{
	m_line += str;
	return *this;
}


Debug& Debug::operator<<(const std::string& str)
{
	m_line += str;
	return *this;
}


Debug& Debug::operator<<(short l)
{
	m_line += Utility::l2string(l);
	return *this;
}


Debug& Debug::operator<<(int l)
{
	m_line += Utility::l2string(l);
	return *this;
}


Debug& Debug::operator<<(long l)
{
	m_line += Utility::l2string(l);
	return *this;
}


Debug& Debug::operator<<(double d)
{
	char slask[100];
	snprintf(slask, sizeof(slask), "%f", d);
	m_line += slask;
	return *this;
}


Debug& Debug::operator<<(endl)
{
	Print("%s", m_line.c_str());
	m_line = "";
	return *this;
}


#ifdef SOCKETS_NAMESPACE
} // namespace SOCKETS_NAMESPACE {
#endif

