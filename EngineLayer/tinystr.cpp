/*
www.sourceforge.net/projects/tinyxml
Original file by Yves Berquin.

This software is provided 'as-is', without any express or implied 
warranty. In no event will the authors be held liable for any 
damages arising from the use of this software.

Permission is granted to anyone to use this software for any 
purpose, including commercial applications, and to alter it and 
redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must 
not claim that you wrote the original software. If you use this 
software in a product, an acknowledgment in the product documentation 
would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source 
distribution.
*/

#include "stdafx.h"
#include "tinyxml.h"

#ifndef TIXML_USE_STL

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "tinystr.h"



	// TiXmlString constructor, based on a C string
	TiXmlString::TiXmlString (const WCHAR* instring)
	{
		unsigned newlen;
		WCHAR * newstring;

		if (!instring)
		{
			allocated = 0;
			cstring = NULL;
			current_length = 0;
			return;
		}
		newlen = wcslen (instring) + 1;
		newstring = new WCHAR [newlen];

		CopyMemory(newstring, instring, sizeof(WCHAR)*newlen);
		// strcpy (newstring, instring);

		allocated = newlen;
		cstring = newstring;
		current_length = newlen - 1;
	}

	// TiXmlString copy constructor
	TiXmlString::TiXmlString (const TiXmlString& copy)
	{
		unsigned newlen;
		WCHAR * newstring;

		// Prevent copy to self!
		if ( &copy == this )
			return;

		if (! copy.allocated)
		{
			allocated = 0;
			cstring = NULL;
			current_length = 0;
			return;
		}
		newlen = copy.length () + 1;
		newstring = new WCHAR [newlen];

		// strcpy (newstring, copy.cstring);
		CopyMemory(newstring, copy.cstring, sizeof(WCHAR)*newlen);

		allocated = newlen;
		cstring = newstring;
		current_length = newlen - 1;
	}

	// TiXmlString = operator. Safe when assign own content
	void TiXmlString ::operator = (const WCHAR * content)
	{
		unsigned newlen;
		WCHAR * newstring;

		if (! content)
		{
			empty_it ();
			return;
		}
		newlen = wcslen (content) + 1;
		newstring = new WCHAR [newlen];
		// strcpy (newstring, content);
		CopyMemory(newstring, content, sizeof(WCHAR)*newlen);
		empty_it ();
		allocated = newlen;
		cstring = newstring;
		current_length = newlen - 1;
	}

	// = operator. Safe when assign own content
	void TiXmlString ::operator = (const TiXmlString & copy)
	{
		unsigned newlen;
		WCHAR * newstring;

		if (! copy . length ())
		{
			empty_it ();
			return;
		}
		newlen = copy . length () + 1;
		newstring = new WCHAR [newlen];
		// strcpy (newstring, copy.c_str ());
		CopyMemory(newstring, copy.c_str (), sizeof(WCHAR)*newlen);
		empty_it ();
		allocated = newlen;
		cstring = newstring;
		current_length = newlen - 1;
	}


	//// Checks if a TiXmlString contains only whitespace (same rules as isspace)
	//BOOL TiXmlString::isblank () const
	//{
	//    WCHAR * lookup;
	//    for (lookup = cstring; * lookup; lookup++)
	//        if (! isspace (* lookup))
	//            return FALSE;
	//    return TRUE;
	//}

	// append a const WCHAR * to an existing TiXmlString
	void TiXmlString::append( const WCHAR* str, int len )
	{
		WCHAR * new_string;
		unsigned new_alloc, new_size, size_suffix;

		size_suffix = wcslen (str);
		if (len < (int) size_suffix)
			size_suffix = len;
		if (! size_suffix)
			return;

		new_size = length () + size_suffix + 1;
		// check if we need to expand
		if (new_size > allocated)
		{
			// compute new size
			new_alloc = assign_new_size (new_size);

			// allocate new buffer
			new_string = new WCHAR [new_alloc];        
			new_string [0] = 0;

			// copy the previous allocated buffer into this one
			if (allocated && cstring)
				// strcpy (new_string, cstring);
				CopyMemory (new_string, cstring, sizeof(WCHAR)*length ());

			// append the suffix. It does exist, otherwize we wouldn't be expanding 
			// strncat (new_string, str, len);
			CopyMemory (new_string + length (), 
					str,
					sizeof(WCHAR)*size_suffix);

			// return previsously allocated buffer if any
			if (allocated && cstring)
				delete [] cstring;

			// update member variables
			cstring = new_string;
			allocated = new_alloc;
		}
		else
		{
			// we know we can safely append the new string
			// strncat (cstring, str, len);
			CopyMemory (cstring + length (), 
					str,
					sizeof(WCHAR)*size_suffix);
		}
		current_length = new_size - 1;
		cstring [current_length] = 0;
	}


	// append a const WCHAR * to an existing TiXmlString
	void TiXmlString::append( const WCHAR * suffix )
	{
		WCHAR * new_string;
		unsigned new_alloc, new_size;

		new_size = length () + wcslen (suffix) + 1;
		// check if we need to expand
		if (new_size > allocated)
		{
			// compute new size
			new_alloc = assign_new_size (new_size);

			// allocate new buffer
			new_string = new WCHAR [new_alloc];        
			new_string [0] = 0;

			// copy the previous allocated buffer into this one
			if (allocated && cstring)
				CopyMemory (new_string, cstring, sizeof(WCHAR)*(1 + length ()));
				// strcpy (new_string, cstring);

			// append the suffix. It does exist, otherwize we wouldn't be expanding 
			// strcat (new_string, suffix);
			CopyMemory (new_string + length (), 
					suffix,
					sizeof(WCHAR)*(wcslen (suffix) + 1));

			// return previsously allocated buffer if any
			if (allocated && cstring)
				delete [] cstring;

			// update member variables
			cstring = new_string;
			allocated = new_alloc;
		}
		else
		{
			// we know we can safely append the new string
			// strcat (cstring, suffix);
			CopyMemory (cstring + length (), 
					suffix, 
					sizeof(WCHAR)*(wcslen (suffix) + 1));
		}
		current_length = new_size - 1;
	}

	// Check for TiXmlString equuivalence
	//BOOL TiXmlString::operator == (const TiXmlString & compare) const
	//{
	//    return (! strcmp (c_str (), compare . c_str ()));
	//}

	//unsigned TiXmlString::length () const
	//{
	//    if (allocated)
	//        // return strlen (cstring);
	//        return current_length;
	//    return 0;
	//}


	unsigned TiXmlString::find (WCHAR tofind, unsigned offset) const
	{
		WCHAR * lookup;

		if (offset >= length ())
			return (unsigned) notfound;
		for (lookup = cstring + offset; * lookup; lookup++)
			if (* lookup == tofind)
				return lookup - cstring;
		return (unsigned) notfound;
	}


	BOOL TiXmlString::operator == (const TiXmlString & compare) const
	{
		if ( allocated && compare.allocated )
		{
			assert( cstring );
			assert( compare.cstring );
			return ( wcscmp( cstring, compare.cstring ) == 0 );
 		}
		return FALSE;
	}


	BOOL TiXmlString::operator < (const TiXmlString & compare) const
	{
		if ( allocated && compare.allocated )
		{
			assert( cstring );
			assert( compare.cstring );
			return ( wcscmp( cstring, compare.cstring ) > 0 );
 		}
		return FALSE;
	}


	BOOL TiXmlString::operator > (const TiXmlString & compare) const
	{
		if ( allocated && compare.allocated )
		{
			assert( cstring );
			assert( compare.cstring );
			return ( wcscmp( cstring, compare.cstring ) < 0 );
 		}
		return FALSE;
	}



#endif	// TIXML_USE_STL
