/**
 **	\file Json.cpp
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
#include "Json.h"
#include <sstream>
#include "Utility.h"
#include <cstdio>

#ifdef _WIN32
#ifdef GetObject
#undef GetObject
#endif
#endif

#define C buffer[index]

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif


// --------------------------------------------------------------------------------
Json::Json() : m_type(TYPE_UNKNOWN)
{
}


// --------------------------------------------------------------------------------
Json::Json(char value) : m_type(TYPE_INTEGER), m_i_value(value)
{
}


// --------------------------------------------------------------------------------
Json::Json(short value) : m_type(TYPE_INTEGER), m_i_value(value)
{
}


// --------------------------------------------------------------------------------
Json::Json(long value) : m_type(TYPE_INTEGER), m_i_value(value)
{
}


// --------------------------------------------------------------------------------
Json::Json(double value) : m_type(TYPE_REAL), m_d_value(value)
{
}


// --------------------------------------------------------------------------------
Json::Json(const char *value) : m_type(TYPE_STRING), m_str_value(value)
{
}


// --------------------------------------------------------------------------------
Json::Json(const std::string& value) : m_type(TYPE_STRING), m_str_value(value)
{
}


// --------------------------------------------------------------------------------
Json::Json(bool value) : m_type(TYPE_BOOLEAN), m_b_value(value)
{
}


// --------------------------------------------------------------------------------
Json::Json(json_type_t t) : m_type(t)
{
	if (t != TYPE_ARRAY && t != TYPE_OBJECT)
		throw Exception("Must be type: Array or type: Object");
}


// --------------------------------------------------------------------------------
Json::~Json()
{
}


// --------------------------------------------------------------------------------
Json::json_type_t Json::Type() const { return m_type; }


// --------------------------------------------------------------------------------
bool Json::HasValue(const std::string& name) const
{
	if (m_type != TYPE_OBJECT)
		return false;
	return m_object.find(name) != m_object.end();
}


// --------------------------------------------------------------------------------
Json::operator char() const
{
	return (char)m_i_value;
}


// --------------------------------------------------------------------------------
Json::operator short() const
{
	return (short)m_i_value;
}


// --------------------------------------------------------------------------------
Json::operator long() const
{
	return m_i_value;
}


// --------------------------------------------------------------------------------
Json::operator double() const
{
	return m_d_value;
}


// --------------------------------------------------------------------------------
Json::operator std::string() const
{
	return m_str_value;
}


// --------------------------------------------------------------------------------
Json::operator bool() const
{
	return m_b_value;
}


// --------------------------------------------------------------------------------
const Json& Json::operator[](const char *name) const
{
	if (m_type != TYPE_OBJECT)
		throw Exception("Must be type: Object");
	json_map_t::const_iterator it = m_object.find(name);
	if (it != m_object.end())
		return it -> second;
	throw Exception("Key not found: " + std::string(name));
}

// --------------------------------------------------------------------------------
Json& Json::operator[](const char *name)
{
	if (m_type == TYPE_UNKNOWN)
		m_type = TYPE_OBJECT;
	if (m_type != TYPE_OBJECT)
		throw Exception("Must be type: Object");
	return m_object[name];
}


// --------------------------------------------------------------------------------
const Json& Json::operator[](const std::string& name) const
{
	if (m_type != TYPE_OBJECT)
		throw Exception("Must be type: Object");
	json_map_t::const_iterator it = m_object.find(name);
	if (it != m_object.end())
		return it -> second;
	throw Exception("Key not found: " + name);
}

// --------------------------------------------------------------------------------
Json& Json::operator[](const std::string& name)
{
	if (m_type == TYPE_UNKNOWN)
		m_type = TYPE_OBJECT;
	if (m_type != TYPE_OBJECT)
		throw Exception("Must be type: Object");
	return m_object[name];
}


// --------------------------------------------------------------------------------
void Json::Add(Json data)
{
	if (m_type == TYPE_UNKNOWN)
		m_type = TYPE_ARRAY;
	if (m_type != TYPE_ARRAY)
		throw Exception("trying to add array data in non-array");
	m_array.push_back(data);
}


// --------------------------------------------------------------------------------
const std::string& Json::GetString() const
{
	return m_str_value;
}


// --------------------------------------------------------------------------------
Json::json_list_t& Json::GetArray()
{
	if (m_type == TYPE_UNKNOWN)
		m_type = TYPE_ARRAY;
	if (m_type != TYPE_ARRAY)
		throw Exception("Json instance not of type: Array");
	return m_array;
}


// --------------------------------------------------------------------------------
Json::json_map_t& Json::GetObject()
{
	if (m_type == TYPE_UNKNOWN)
		m_type = TYPE_OBJECT;
	if (m_type != TYPE_OBJECT)
		throw Exception("Json instance not of type: Array");
	return m_object;
}


// --------------------------------------------------------------------------------
const Json::json_list_t& Json::GetArray() const
{
	if (m_type != TYPE_ARRAY)
		throw Exception("Json instance not of type: Array");
	return m_array;
}


// --------------------------------------------------------------------------------
const Json::json_map_t& Json::GetObject() const
{
	if (m_type != TYPE_OBJECT)
		throw Exception("Json instance not of type: Array");
	return m_object;
}


// --------------------------------------------------------------------------------
Json Json::Parse(const std::string& data)
{
	size_t i = 0;
	Json obj;
	obj.Parse(data.c_str(), i);
	return obj;
}


// --------------------------------------------------------------------------------
int Json::Token(const char *buffer, size_t& index, std::string& ord)
{
	while (C == ' ' || C == 9 || C == 13 || C == 10)
		++index;
	size_t x = index; // origin
	if (C == '-' || isdigit(C)) // Number
	{
		bool dot = false;
		if (C == '-')
		{
			++index;
		}
		while (isdigit(C) || C == '.')
		{
			if (C == '.')
				dot = true;
			++index;
		}
		size_t sz = index - x;
		ord = std::string(buffer + x, sz);
		if (dot)
		{
			m_type = TYPE_REAL;
		}
		else
		{
			m_type = TYPE_INTEGER;
		}
		return -m_type;
	}
	else
	if (C == 34) // " - String
	{
		bool ign = false;
		x = ++index;
		while (C && (ign || C != 34))
		{
			if (ign)
			{
				ign = false;
			}
			else
			if (C == '\\')
			{
				ign = true;
			}
			++index;
		}
		size_t sz = index - x;
		ord = std::string(buffer + x, sz);
		decode(ord);
		++index;
		m_type = TYPE_STRING;
		return -m_type;
	}
	else
	if (!strncmp(&buffer[index], "null", 4)) // null value
	{
		m_type = TYPE_UNKNOWN;
		ord = std::string(buffer + x, 4);
		index += 4;
		return -m_type;
	}
	else
	if (!strncmp(&buffer[index], "true", 4)) // Boolean: true
	{
		m_type = TYPE_BOOLEAN;
		ord = std::string(buffer + x, 4);
		m_b_value = true;
		index += 4;
		return -m_type;
	}
	else
	if (!strncmp(&buffer[index], "false", 5)) // Boolean: false
	{
		m_type = TYPE_BOOLEAN;
		ord = std::string(buffer + x, 5);
		m_b_value = false;
		index += 5;
		return -m_type;
	}
	return buffer[index++];
}


// --------------------------------------------------------------------------------
char Json::Parse(const char *buffer, size_t& index)
{
	std::string ord;
	int token = Token(buffer, index, ord);
	if (token == -TYPE_REAL || token == -TYPE_INTEGER)
	{
		if (token == -TYPE_REAL)
		{
			m_d_value = atof(ord.c_str());
		}
		else
		{
			m_i_value = atoi(ord.c_str());
		}
	}
	else
	if (token == -TYPE_STRING)
	{
		m_str_value = ord;
	}
	else
	if (token == -TYPE_UNKNOWN)
	{
	}
	else
	if (token == -TYPE_BOOLEAN)
	{
	}
	else
	if (token == '[') // Array
	{
		m_type = TYPE_ARRAY;
		while (true)
		{
			char res;
			Json o;
			if ((res = o.Parse(buffer, index)) == 0)
			{
				m_array.push_back(o);
			}
			else
			if (res == ']')
			{
				break;
			}
			else
			if (res == ',') // another element follows
			{
			}
			else
			{
				throw Exception(std::string("Unexpected end of Array: ") + res);
			}
		}
	}
	else
	if (token == ']') // end of Array
	{
		return ']';
	}
	else
	if (token == '{') // Object
	{
		m_type = TYPE_OBJECT;
		int state = 0;
		std::string element_name;
		bool quit = false;
		while (!quit)
		{
			Json o;
			char res = o.Parse(buffer, index);
			switch (state)
			{
			case 0:
				if (res == ',') // another object follow
					break;
				if (res == '}') // end of Object
				{
					quit = true;
					break;
				}
				if (res || o.Type() != TYPE_STRING)
					throw Exception("Object element name missing");
				element_name = o.GetString();
				state = 1;
				break;
			case 1:
				if (res != ':')
					throw Exception("Object element separator missing");
				state = 2;
				break;
			case 2:
				if (res)
					throw Exception(std::string("Unexpected character when parsing anytype: ") + res);
				m_object[element_name] = o;
				state = 0;
				break;
			}
		}
	}
	else
	if (token == '}') // end of Object
	{
		return '}';
	}
	else
	if (token == ',')
	{
		return ',';
	}
	else
	if (token == ':')
	{
		return ':';
	}
	else
	{
		throw Exception("Can't parse Json representation: " + std::string(&buffer[index]));
	}
	return 0;
}


// --------------------------------------------------------------------------------
std::string Json::ToString(bool quote) const
{
	switch (m_type)
	{
	case TYPE_UNKNOWN:
		break;
	case TYPE_INTEGER:
		return Utility::l2string(m_i_value);
	case TYPE_REAL:
	{
		char slask[100];
		sprintf(slask, "%f", m_d_value);
		return slask;
	}
	case TYPE_STRING:
	{
		std::ostringstream tmp;
		tmp << "\"" << encode(m_str_value) << "\"";
		return tmp.str();
	}
	case TYPE_BOOLEAN:
		return m_b_value ? "true" : "false";
	case TYPE_ARRAY:
	{
		std::ostringstream tmp;
		bool first = true;
		tmp << "[";
		for (json_list_t::const_iterator it = m_array.begin(); it != m_array.end(); it++)
		{
			const Json& ref = *it;
			if (!first)
				tmp << ",";
			tmp << ref.ToString(quote);
			first = false;
		}
		tmp << "]";
		return tmp.str();
	}
	case TYPE_OBJECT:
	{
		std::ostringstream tmp;
		bool first = true;
		tmp << "{";
		for (json_map_t::const_iterator it = m_object.begin(); it != m_object.end(); it++)
		{
			const std::pair<std::string, Json>& ref = *it;
			if (!first)
				tmp << ",";
			if (quote)
				tmp << "\"" << encode(ref.first) << "\":" << ref.second.ToString(quote);
			else
				tmp << ref.first << ":" << ref.second.ToString(quote);
			first = false;
		}
		tmp << "}";
		return tmp.str();
	}
	}
	return "null";
}


// --------------------------------------------------------------------------------
void Json::encode(std::string& src) const
{
	size_t pos = src.find("\\");
	while (pos != std::string::npos)
	{
		src.replace(pos, 1, "\\\\");
		pos = src.find("\\", pos + 2);
	}
	pos = src.find("\r");
	while (pos != std::string::npos)
	{
		src.replace(pos, 1, "\\r");
		pos = src.find("\r", pos + 2);
	}
	pos = src.find("\n");
	while (pos != std::string::npos)
	{
		src.replace(pos, 1, "\\n");
		pos = src.find("\n", pos + 2);
	}
	pos = src.find("\"");
	while (pos != std::string::npos)
	{
		src.replace(pos, 1, "\\\"");
		pos = src.find("\"", pos + 2);
	}
}


// --------------------------------------------------------------------------------
void Json::decode(std::string& src) const
{
	size_t pos = src.find("\\\"");
	while (pos != std::string::npos)
	{
		src.replace(pos, 2, "\"");
		pos = src.find("\\\"", pos + 1);
	}
	pos = src.find("\\r");
	while (pos != std::string::npos)
	{
		src.replace(pos, 2, "\r");
		pos = src.find("\\r", pos + 1);
	}
	pos = src.find("\\n");
	while (pos != std::string::npos)
	{
		src.replace(pos, 2, "\n");
		pos = src.find("\\n", pos + 1);
	}
	pos = src.find("\\\\");
	while (pos != std::string::npos)
	{
		src.replace(pos, 2, "\\");
		pos = src.find("\\\\", pos + 1);
	}
}


// --------------------------------------------------------------------------------
std::string Json::encode(const std::string& src) const
{
	std::string tmp(src);
	encode(tmp);
	return tmp;
}


#ifdef SOCKETS_NAMESPACE
} // namespace SOCKETS_NAMESPACE {
#endif

