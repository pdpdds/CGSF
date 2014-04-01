/** \file File.cpp
 **	\date  2005-04-25
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
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "File.h"

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif


File::File()
:m_fil(NULL)
,m_b_close(true)
,m_rptr(0)
,m_wptr(0)
{
}


File::File(FILE *fil)
:m_fil(fil)
,m_b_close(false)
,m_rptr(0)
,m_wptr(0)
{
}


File::File(const std::string& path, const std::string& mode)
:m_fil(NULL)
,m_b_close(true)
,m_rptr(0)
,m_wptr(0)
{
	fopen(path, mode);
}


File::~File()
{
	if (m_b_close)
	{
		fclose();
	}
}


bool File::fopen(const std::string& path, const std::string& mode)
{
	m_path = path;
	m_mode = mode;
#if defined( _WIN32) && !defined(__CYGWIN__)
	if (fopen_s(&m_fil, path.c_str(), mode.c_str()))
		m_fil = NULL;
#else
	m_fil = ::fopen(path.c_str(), mode.c_str());
#endif

	return m_fil ? true : false;
}


void File::fclose() const
{
	if (m_fil)
	{
		::fclose(m_fil);
		m_fil = NULL;
	}
}



size_t File::fread(char *ptr, size_t size, size_t nmemb) const
{
	size_t r = 0;
	if (m_fil)
	{
		fseek(m_fil, m_rptr, SEEK_SET);
		r = ::fread(ptr, size, nmemb, m_fil);
		m_rptr = ftell(m_fil);
	}
	return r;
}


size_t File::fwrite(const char *ptr, size_t size, size_t nmemb)
{
	size_t r = 0;
	if (m_fil)
	{
		fseek(m_fil, m_wptr, SEEK_SET);
		r = ::fwrite(ptr, size, nmemb, m_fil);
		m_wptr = ftell(m_fil);
	}
	return r;
}



char *File::fgets(char *s, int size) const
{
	char *r = NULL;
	if (m_fil)
	{
		fseek(m_fil, m_rptr, SEEK_SET);
		r = ::fgets(s, size, m_fil);
		m_rptr = ftell(m_fil);
	}
	return r;
}


void File::fprintf(const char *format, ...)
{
	if (!m_fil)
		return;
	va_list ap;
	va_start(ap, format);
	fseek(m_fil, m_wptr, SEEK_SET);
	vfprintf(m_fil, format, ap);
	m_wptr = ftell(m_fil);
	va_end(ap);
}


off_t File::size() const
{
	struct stat st;
	if (stat(m_path.c_str(), &st) == -1)
	{
		return 0;
	}
	return st.st_size;
}


bool File::eof() const
{
	if (m_fil)
	{
		if (feof(m_fil))
			return true;
	}
	return false;
}


void File::reset_read() const
{
	m_rptr = 0;
}


void File::reset_write()
{
	m_wptr = 0;
}


const std::string& File::Path() const
{
	return m_path;
}


#ifdef SOCKETS_NAMESPACE
}
#endif


