/**
 **	\file XmlDocument.cpp
 **	\date  2008-02-09
 **	\author grymse@alhem.net
**/
/*
Copyright (C) 2008-2011  Anders Hedstrom

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
#include "XmlDocument.h"
#include "XmlException.h"

#ifdef ENABLE_XML

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif


XmlDocument::XmlDocument(const std::string& filename,const std::string& verify_ns,const std::string& verify_root)
: m_doc(NULL)
, m_ok(false)
{
	xmlNodePtr cur;
	xmlNsPtr ns;

	if (!(m_doc = xmlParseFile(filename.c_str() )))
	{
		throw XmlException("Parse of file failed: " + filename);
	}
	if (!(cur = xmlDocGetRootElement(m_doc)))
	{
		xmlFreeDoc(m_doc);
		m_doc = NULL;
		throw XmlException("Document is empty: " + filename);
	}
	if (verify_ns.size())
	{
		if (!(ns = xmlSearchNsByHref(m_doc, cur, (const xmlChar *) verify_ns.c_str() )))
		{
			xmlFreeDoc(m_doc);
			m_doc = NULL;
			throw XmlException("Document namespace != " + verify_ns);
		}
	}
	if (verify_root.size())
	{
		if (xmlStrcmp(cur -> name, (const xmlChar *) verify_root.c_str() ))
		{
			xmlFreeDoc(m_doc);
			m_doc = NULL;
			throw XmlException("Document root != " + verify_root);
		}
	}
	m_ok = true;
}


XmlDocument::~XmlDocument()
{
	if (m_doc)
	{
		xmlFreeDoc(m_doc);
	}
}


XmlDocument::operator xmlDocPtr()
{
	return m_doc;
}


#ifdef SOCKETS_NAMESPACE
}
#endif
#endif // ENABLE_XML
