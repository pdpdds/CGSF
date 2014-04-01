/**
 **	\file XmlNode.cpp
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
#include "XmlNode.h"
#include "XmlDocument.h"
#include "XmlException.h"
#include "Utility.h"

#ifdef ENABLE_XML

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif


XmlNode::XmlNode(XmlDocument& doc)
: m_doc(doc)
, m_current( GetRootElement() )
{
}


XmlNode::XmlNode(XmlDocument& doc, const std::string& nodepath)
: m_doc(doc)
, m_current( GetRootElement() )
{
	xmlNodePtr p = GetFirstElement( nodepath );
	SetCurrent( p );
}


XmlNode::XmlNode(const XmlNode& node, const std::string& nodepath)
: m_doc( node.GetDocument() )
, m_current( node )
{
	xmlNodePtr p = GetFirstElement( node, nodepath );
	SetCurrent( p );
}


XmlNode::XmlNode(XmlDocument& doc, xmlNodePtr ptr)
: m_doc(doc)
, m_current( ptr )
{
}


XmlNode::XmlNode(xmlDocPtr doc, xmlNodePtr ptr)
: m_doc(doc)
, m_current( ptr )
{
}


XmlNode::~XmlNode()
{
}


xmlNodePtr XmlNode::GetRootElement() const
{
	m_current = xmlDocGetRootElement(m_doc);
	return m_current;
}


std::string XmlNode::GetProperty(const std::string& name) const
{
	xmlChar *p = m_current ? xmlGetProp(m_current, (const xmlChar *) name.c_str() ) : NULL;
	if (!p)
	{
		throw XmlException( "Property '" + name + "' not found in node: " + GetNodeName() );
	}
	std::string str = (char *)p;
	xmlFree(p);
	return Utility::FromUtf8(str);
}


bool XmlNode::PropertyExists(const std::string& name) const
{
	xmlChar *p = m_current ? xmlGetProp(m_current, (const xmlChar *) name.c_str() ) : NULL;
	if (!p)
	{
		return false;
	}
	xmlFree(p);
	return true;
}


xmlNodePtr XmlNode::GetChildrenNode() const
{
	m_current = m_current ? m_current -> xmlChildrenNode : NULL;
	return m_current;
}


xmlNodePtr XmlNode::GetNextNode() const
{
	do
	{
		m_current = m_current ? m_current -> next : NULL;
	} while (m_current && xmlIsBlankNode( m_current ));
	return m_current;
}


const std::string& XmlNode::GetNodeName() const
{
	if (m_current)
	{
		m_current_name = Utility::FromUtf8((char *)m_current -> name);
	}
	else
	{
		m_current_name = "";
	}
	return m_current_name;
}


const std::string& XmlNode::GetContent() const
{
	m_content = "";
	if (m_current)
	{
		xmlNodePtr p = m_current;
		xmlNodePtr p2 = GetChildrenNode();
		if (p2 && p2 -> content)
		{
			m_content = Utility::FromUtf8((char *)p2 -> content);
		}
		SetCurrent(p);
	}
	return m_content;
}


xmlNsPtr XmlNode::GetNodeNs() const
{
	if (m_current)
		return m_current -> ns;
	return NULL;
}


const std::string& XmlNode::GetNodeNsPrefix() const
{
	if (m_current && m_current -> ns && m_current -> ns -> prefix)
	{
		m_ns_prefix = Utility::FromUtf8((char *)m_current -> ns -> prefix);
	}
	else
	{
		m_ns_prefix = "";
	}
	return m_ns_prefix;
}


const std::string& XmlNode::GetNodeNsHref() const
{
	if (m_current && m_current -> ns && m_current -> ns -> href)
	{
		m_ns_href = Utility::FromUtf8((char *)m_current -> ns -> href);
	}
	else
	{
		m_ns_href = "";
	}
	return m_ns_href;
}


xmlNodePtr XmlNode::GetFirstElement(const std::string& name) const
{
	if (m_lookup_name.empty())
		m_lookup_name = name;
	GetRootElement();
	xmlNodePtr p = GetChildrenNode();
	while (p)
	{
		if (name == GetNodeName())
		{
			return p;
		}
		p = GetNextNode();
	}
	return NULL;
}


xmlNodePtr XmlNode::GetFirstElement(xmlNodePtr base,const std::string& name) const
{
	if (m_lookup_name.empty())
		m_lookup_name = name;
	SetCurrent(base);
	xmlNodePtr p = GetChildrenNode();
	while (p)
	{
		if (name == GetNodeName())
		{
			return p;
		}
		p = GetNextNode();
	}
	return NULL;
}


xmlNodePtr XmlNode::GetNextElement(xmlNodePtr p,const std::string& name) const
{
	SetCurrent(p);
	p = GetNextNode();
	while (p)
	{
		if (name == GetNodeName())
		{
			return p;
		}
		p = GetNextNode();
	}
	return NULL;
}


XmlNode::operator xmlNodePtr() const
{
	return m_current;
}


XmlNode XmlNode::operator[](const std::string& name) const
{
	xmlNodePtr p0 = m_current;
	xmlNodePtr p = GetFirstElement( m_current, name );
	SetCurrent( p0 );
	if (p)
		return XmlNode( m_doc, p );
	throw XmlException("Didn't find node: " + name);
}


bool XmlNode::Exists(const std::string& name) const
{
	xmlNodePtr p0 = m_current;
	xmlNodePtr p = GetFirstElement( m_current, name );
	SetCurrent( p0 );
	if (p)
		return true;
	return false;
}


void XmlNode::operator++() const
{
	GetNextNode();
	while (m_current)
	{
		if (m_lookup_name == GetNodeName())
		{
			return;
		}
		GetNextNode();
	}
}


std::map<std::string, std::string> XmlNode::GetNsMap() const
{
	xmlNsPtr *p = xmlGetNsList(m_doc, m_current);
	std::map<std::string, std::string> vec;
	int i = 0;
	while (p[i])
	{
		std::string href = Utility::FromUtf8((char *)p[i] -> href);
		std::string prefix = p[i] -> prefix ? Utility::FromUtf8((char *)p[i] -> prefix) : "";
		vec[prefix] = href;
		++i;
	}
	return vec;
}


std::map<std::string, std::string> XmlNode::GetNsMapRe() const
{
	xmlNsPtr *p = xmlGetNsList(m_doc, m_current);
	std::map<std::string, std::string> vec;
	int i = 0;
	while (p[i])
	{
		std::string href = Utility::FromUtf8((char *)p[i] -> href);
		std::string prefix = p[i] -> prefix ? Utility::FromUtf8((char *)p[i] -> prefix) : "";
		vec[href] = prefix;
		if (!p[i] -> next)
			break;
		++i;
	}
	return vec;
}


const std::string XmlNode::FindProperty(const std::string& propname, bool climb) const
{
	while (m_current)
	{
		if (PropertyExists(propname))
		{
			return GetProperty(propname);
		}
		if (!climb)
		{
			break;
		}
		m_current = m_current -> parent;
	}
	return "";
}


#ifdef SOCKETS_NAMESPACE
}
#endif

#endif // ENABLE_XML
