/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2010 The SWG:ANH Team
---------------------------------------------------------------------------------------
Use of this source code is governed by the GPL v3 license that can be found
in the COPYING file or at http://www.gnu.org/licenses/gpl-3.0.html

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
---------------------------------------------------------------------------------------
*/

#include "StdAfx.h"
#include "SFFString.h"


#include <algorithm>

#include <cassert>
#include <cctype>
#include <cstdlib>
#include <cstring>

#include <wchar.h>
#include <wctype.h>

//======================================================================================================================
#pragma warning(disable : 4996)
SFFString::SFFString()
    : m_szBuffer(0)
    , m_Type(SFFSTRType_ANSI)
    , m_Allocated(0)
    , m_CharacterWidth(1)
    , m_Length(0)
{
    _allocate();
    *(DWORD*)m_szBuffer = 0;   // Make sure a new empty string is null terminated.
}

//======================================================================================================================

SFFString::SFFString(SFFStringType type,USHORT length)
    : m_szBuffer(0)
    , m_Type(type)
    , m_Allocated(0)
    , m_Length(length)
{
    if(type == SFFSTRType_ANSI)
        m_CharacterWidth = 1;
    else if(type == SFFSTRType_Unicode16)
        m_CharacterWidth = 2;

    _allocate();
    *(DWORD*)m_szBuffer = 0;
}

//======================================================================================================================

SFFString::~SFFString()
{
    if (m_szBuffer) delete [] m_szBuffer;
}

//======================================================================================================================

SFFString::SFFString(const char* data)
    : m_szBuffer(0)
    , m_Type(SFFSTRType_ANSI)
    , m_Allocated(0)
    , m_CharacterWidth(1)
    , m_Length(0)
{
    _allocate();
    // we might get a null pointer from db queries
    if(data != NULL)
        *this = data;
    else
        *(DWORD*)m_szBuffer = 0;
}

//======================================================================================================================

SFFString::SFFString(const USHORT* data)
    : m_szBuffer(0)
    , m_Type(SFFSTRType_Unicode16)
    , m_Allocated(0)
    , m_CharacterWidth(2)
    , m_Length(0)
{
    _allocate();
    // we might get a null pointer from db queries
    if(data != NULL)
        *this = data;
    else
        *(DWORD*)m_szBuffer = 0;
}

//======================================================================================================================

SFFString::SFFString(const wchar_t* data)
    : m_szBuffer(0)
    , m_Type(SFFSTRType_Unicode16)
    , m_Allocated(0)
    , m_CharacterWidth(2)
    , m_Length(0)
{
    _allocate();
    // we might get a null pointer from db queries
    if(data != NULL)
        *this = data;
    else
        *(DWORD*)m_szBuffer = 0;
}


//======================================================================================================================

SFFString::SFFString(const SFFString& data)
    : m_szBuffer(0)
    , m_Type(SFFSTRType_ANSI)
    , m_Allocated(0)
    , m_CharacterWidth(1)
    , m_Length(0)
{
    _allocate();
    *this = data;
}

//======================================================================================================================

USHORT SFFString::initRawSFFSTR(char* data, SFFStringType type)
{
    USHORT totalLen = *(USHORT*)data;

    m_Type = type;

    switch(type)
    {
    case SFFSTRType_ANSI:
    {
        m_CharacterWidth = 1;
    }
    break;

    case SFFSTRType_Unicode16:
    case SFFSTRType_UTF8:
    {
        m_CharacterWidth = 2;
    }
    break;
    }

    DWORD charLen = (totalLen + 1) * m_CharacterWidth;

    // If we don't have enough room in our buffer, re-allocate a new one
    if(charLen > m_Allocated)
    {
        if(m_szBuffer)
            delete [] m_szBuffer;

        m_Allocated = (((static_cast<USHORT>(charLen) / SFFString_ALLOC_BLOCK_SIZE) + 1) * SFFString_ALLOC_BLOCK_SIZE);
        m_szBuffer = new char[m_Allocated];

        memset(m_szBuffer,0,m_Allocated);
    }

    // Copy our string into the buffer.
    if(m_Allocated)
    {
        memcpy(m_szBuffer,data+(m_CharacterWidth*2),charLen - m_CharacterWidth);
        memset(&m_szBuffer[charLen - m_CharacterWidth], 0, m_CharacterWidth);
    }

    m_Length = totalLen;

    return totalLen;
}

//======================================================================================================================

bool SFFString::operator ==(char* data) // compare against ansi null terminated string
{
    return(strcmp(data,m_szBuffer) == 0);
}

//======================================================================================================================

bool SFFString::operator ==(const char* data) // compare against const ansi null terminated string
{
    return(strcmp(data,m_szBuffer) == 0);
}

//======================================================================================================================

bool SFFString::operator ==(const SFFString& data)
{
    return (strcmp(m_szBuffer, data.getAnsi()) == 0);
}

//======================================================================================================================

bool SFFString::operator ==(SFFString data)
{
    return (strcmp(m_szBuffer, data.getAnsi()) == 0);
}

//======================================================================================================================

SFFString& SFFString::operator =(const char* data)
{
    // we might get a null pointer from db queries
    if(data != NULL)
    {
        m_Type = SFFSTRType_ANSI;
        m_CharacterWidth = 1;
        m_Length = (USHORT)strlen(data);

        // If we don't have enough room in our buffer, re-allocate a new one
        _allocate();

        // Cpoy our string into the new buffer.
        if (m_Allocated)
        {
            memcpy(m_szBuffer, data, m_Length);
            memset(&m_szBuffer[m_Length], 0, 1);
        }
    }

    return *this;
}

//======================================================================================================================
SFFString& SFFString::operator =(const USHORT* data)
{
    // we might get a null pointer from db queries
    if(data != NULL)
    {
        m_Type = SFFSTRType_Unicode16;
        m_CharacterWidth = 2;
        m_Length = static_cast<USHORT>(wcslen(reinterpret_cast<const wchar_t*>(data)));

        // If we don't have enough room in our buffer, re-allocate a new one
        _allocate();

        // Cpoy our string into the new buffer.
        if (m_Allocated)
        {
            memcpy(m_szBuffer, data, m_Length*2);
            memset(&m_szBuffer[m_Length*2], 0, 1);
        }
    }
    return *this;
}

//======================================================================================================================
SFFString& SFFString::operator =(const wchar_t* data)
{
    // we might get a null pointer from db queries
    if(data != NULL)
    {
        m_Type = SFFSTRType_Unicode16;
        m_CharacterWidth = 2;
        m_Length = static_cast<USHORT>(wcslen(data));

        // If we don't have enough room in our buffer, re-allocate a new one
        _allocate();

        // Cpoy our string into the new buffer.
        if (m_Allocated)
        {
            memcpy(m_szBuffer, data, m_Length*2);
            memset(&m_szBuffer[m_Length*2], 0, 1);
        }
    }
    return *this;
}


//======================================================================================================================

SFFString& SFFString::operator =(char* data)
{
    // we might get a null pointer from db queries
    if(data != NULL)
    {
        m_Type = SFFSTRType_ANSI;
        m_CharacterWidth = 1;
        m_Length = (USHORT)strlen(data);

        _allocate();

        // Copy our string into the new buffer.
        if(m_Allocated)
        {
            memcpy(m_szBuffer, data, m_Length);
            memset(&m_szBuffer[m_Length], 0, 1);
        }
    }

    return *this;
}

//======================================================================================================================

SFFString& SFFString::operator =(const SFFString& data)
{
    m_Type = data.getType();
    m_CharacterWidth = static_cast<USHORT>(data.getCharacterWidth());
    m_Length = data.getLength();
    m_Allocated = static_cast<USHORT>(data.getAllocated());

    if (m_szBuffer)
        delete [] m_szBuffer;
    m_szBuffer = new char[m_Allocated];

    // Copy our string into the new buffer.
    memcpy(m_szBuffer, data.getRawData(), m_Allocated);

    return *this;
}


//======================================================================================================================


SFFString& SFFString::operator <<(const USHORT* data)
{
    if(m_Type != SFFSTRType_Unicode16)
        return *this;

    // we might get a null pointer from db queries
    if(data != NULL)
    {
        // Get our source string length
        USHORT dataLength  = static_cast<USHORT>(wcslen(reinterpret_cast<const wchar_t*>(data)));
        //USHORT dataLength = (USHORT)strlen(data);

        // Set our new length and allocate
        USHORT oldLength = m_Length;
        m_Length += dataLength;
        _allocate();

        // Append our string into the bugger.
        memcpy(m_szBuffer + oldLength * m_CharacterWidth, data, dataLength* m_CharacterWidth);
        memset(m_szBuffer + m_Length * m_CharacterWidth, 0, m_CharacterWidth);
    }
    return *this;
}


SFFString& SFFString::operator <<(USHORT* data)
{
    if(m_Type != SFFSTRType_Unicode16)
        return *this;

    // we might get a null pointer from db queries
    if(data != NULL)
    {
        // Get our source string length
        USHORT dataLength  = static_cast<USHORT>(wcslen(reinterpret_cast<const wchar_t*>(data)));
        //USHORT dataLength = (USHORT)strlen(data);

        // Set our new length and allocate
        USHORT oldLength = m_Length;
        m_Length += dataLength;
        _allocate();

        // Append our string into the bugger.
        memcpy(m_szBuffer + oldLength * m_CharacterWidth, data, dataLength* m_CharacterWidth);
        memset(m_szBuffer + m_Length * m_CharacterWidth, 0, m_CharacterWidth);
    }
    return *this;
}

SFFString& SFFString::operator <<(const char* data)
{
    // we might get a null pointer from db queries
    if(data != NULL)
    {
        // Get our source string length
        USHORT dataLength = (USHORT)strlen(data);

        // Set our new length and allocate
        USHORT oldLength = m_Length;
        m_Length += dataLength;
        _allocate();

        // Append our string into the bugger.
        memcpy(m_szBuffer + oldLength * m_CharacterWidth, data, dataLength);
        memset(m_szBuffer + m_Length * m_CharacterWidth, 0, m_CharacterWidth);
    }
    return *this;
}


//======================================================================================================================
SFFString& SFFString::operator <<(char* data)
{
    // we might get a null pointer from db queries
    if(data != NULL)
    {
        // Get our source string length
        USHORT dataLength = (USHORT)strlen(data);

        // Set our new length and allocate
        USHORT oldLength = m_Length;
        m_Length += dataLength;
        _allocate();

        // Append our string into the bugger.
        memcpy(m_szBuffer + oldLength * m_CharacterWidth, data, dataLength);
        memset((m_szBuffer + m_Length * m_CharacterWidth), 0, m_CharacterWidth);
    }
    return *this;
}

//======================================================================================================================

const char* SFFString::getAnsi() const
{
    if (m_Type == SFFSTRType_ANSI) {
        return m_szBuffer;
    } else {
        return 0;
    }
}

//======================================================================================================================

const wchar_t* SFFString::getUnicode16() const
{
    if (m_Type == SFFSTRType_Unicode16)
    {
        return reinterpret_cast<wchar_t*>(m_szBuffer);
    } else {
        return 0;
    }
}

//======================================================================================================================

void SFFString::convert(SFFStringType type)
{
    // Try not to use this often as it is slow.
    // If we're trying to convert to the same type, just return.
    if(m_Type == type)
        return;

    //  Locals
    char*   newBuffer = 0;
    USHORT  allocated = 0;

    // what's the target type and how much space will we need
    switch(type)
    {
    case SFFSTRType_ANSI:
    {
        m_CharacterWidth = 1;
        allocated = ((((m_Length+1) / SFFString_ALLOC_BLOCK_SIZE) + 1) * SFFString_ALLOC_BLOCK_SIZE);

        // Allocate a new buffer for the converted string.
        newBuffer = new char[allocated];
        //Initial null terminator
        memset(newBuffer,0,allocated);

        // Convert the string if needed.
        if(m_Type == SFFSTRType_Unicode16)
        {
			
            wcstombs(newBuffer, reinterpret_cast<wchar_t*>(m_szBuffer),  (((allocated) < (m_Allocated)) ? (allocated) : (m_Allocated)));
        }
        else if(m_Type == SFFSTRType_UTF8)
        {
            // FIXME: Implement
            //int i = 0;
        }
    }
    break;

    case SFFSTRType_Unicode16:
    {
        m_CharacterWidth = 2;
        allocated = (((((m_Length+1) * m_CharacterWidth) / SFFString_ALLOC_BLOCK_SIZE) + 1) * SFFString_ALLOC_BLOCK_SIZE);

        // Allocate a new buffer for the converted string.
        newBuffer = new char[allocated];
        //Initial null terminator
        memset(newBuffer,0,allocated);

        if(m_Type == SFFSTRType_ANSI || m_Type == SFFSTRType_UTF8)
        {
            USHORT* tmp = reinterpret_cast<USHORT*>(newBuffer);
            for (size_t i = 0; i < m_Length; ++i) {
                tmp[i] = static_cast<USHORT>(m_szBuffer[i]);
            }
        }
    }
    break;

    case SFFSTRType_UTF8:
    {
        m_CharacterWidth = 2;

        allocated = (((((m_Length+1) * m_CharacterWidth) / SFFString_ALLOC_BLOCK_SIZE) + 1) * SFFString_ALLOC_BLOCK_SIZE);
        // Allocate a new buffer for the converted string.
        newBuffer = new char[allocated];
        //Initial null terminator
        *(DWORD*)newBuffer = 0;

        if(m_Type == SFFSTRType_ANSI)
        {
            // FIXME: Implement, not sure if it needs to be though
            //int i = 0;
        }
        else if(m_Type == SFFSTRType_Unicode16)
        {
            wcstombs((char*)newBuffer, reinterpret_cast<wchar_t*>(m_szBuffer), m_Length+1);
        }
    }
    break;
    }

    // We are now the new type of string
    if(m_szBuffer != 0)
        delete [] m_szBuffer;

    m_szBuffer		= newBuffer;
    m_Allocated	= allocated;
    m_Type		= type;
}

//======================================================================================================================

// BS code,
// an internal data modifier as toLower(), toUpper() etc ... should NEVER change anything else of the string.
// In this case, the string is FORCED to SFFSTRType_ANSI.

// Implemeting hidden side effects in a function is never good, on a STANDARD object like stings is a disaster.
// Better halt controlled than CTD or server crash.

void SFFString::toLower()
{
    if(m_Type == SFFSTRType_Unicode16)
    {
        assert(false);
        //convert(SFFSTRType_ANSI);
        USHORT* data = (USHORT*)m_szBuffer;

        while(*data)
        {
            *data = towlower(*data);
            data++;
        }
        return;
    }

    char* data = m_szBuffer;

    while(*data)
    {
        *data = tolower(*data);
        ++data;
    }
}

//======================================================================================================================

void SFFString::toUpper()
{
    if(m_Type == SFFSTRType_Unicode16)
    {
        assert(false);
        convert(SFFSTRType_ANSI);
    }

    char* data = m_szBuffer;

    while(*data)
    {
        *data = toupper(*data);
        ++data;
    }
}

//======================================================================================================================

void SFFString::toUpperFirst()
{
    if(m_Type == SFFSTRType_Unicode16)
    {
        USHORT* data = (USHORT*)m_szBuffer;

        *data = towupper(*data);
        return;
    }

    char* data = m_szBuffer;

    *data = toupper(*data);

}

//======================================================================================================================

void SFFString::toLowerFirst()
{
    if(m_Type == SFFSTRType_Unicode16)
    {
        USHORT* data = (USHORT*)m_szBuffer;

        *data = towlower(*data);
        return;
    }

    char* data = m_szBuffer;

    *data = tolower(*data);

}

//======================================================================================================================

bool SFFString::isNumber()
{
    if(m_Type == SFFSTRType_Unicode16)
    {
        assert(false);
        convert(SFFSTRType_ANSI);
    }

    char* data = m_szBuffer;

    while(*data)
    {
        if(!(isdigit(*data)))
            return(false);

        ++data;
    }

    return(true);
}

//======================================================================================================================

int SFFString::split(SFFStringVector& retVec,char delimiter)
{
    retVec.clear();

    if(m_Type == SFFSTRType_Unicode16)
    {
        assert(false);
        convert(SFFSTRType_ANSI);
    }

    char* data = m_szBuffer;
    USHORT beginIndex = 0;
    USHORT endIndex = 0;

    while(1)
    {
        SFFString tmpStr;

        beginIndex = endIndex;

        while(*data != delimiter && *data)
        {
            ++data;
            ++endIndex;
        }

        substring(tmpStr,beginIndex,endIndex);
        tmpStr.getRawData()[endIndex - beginIndex] = 0;

        retVec.push_back(SFFString(tmpStr.getAnsi()));

        if(!*data)
            break;

        if(!*++data)
            break;

        ++endIndex;
    }
    return retVec.size();
}

//======================================================================================================================

void SFFString::substring(SFFString& dest, USHORT start, USHORT end)
{
    // Validate our parameters
    if (start > m_Length || end > m_Length || end < start)
        return;

    // Setup our destination string
    dest.setType(m_Type);
    // dest.setLength(m_Length);
    dest.setLength(end - start);

    // what's the target type and how much space will we need
    switch (m_Type)
    {
    case SFFSTRType_UTF8:
    case SFFSTRType_ANSI:
    {
        char* destBuffer = dest.getRawData();
        strncpy(destBuffer, m_szBuffer + start, end - start);
        break;
    }
    case SFFSTRType_Unicode16:
    {
        USHORT* destBuffer = (USHORT*)dest.getRawData();
        wcsncpy(reinterpret_cast<wchar_t*>(destBuffer), reinterpret_cast<wchar_t*>(m_szBuffer) + start, end - start);
        break;
    }
    }
}

//======================================================================================================================

void SFFString::_allocate()
{
    // If we don't have enough room in our buffer, re-allocate a new one
    if(m_Length * m_CharacterWidth >= m_Allocated)
    {
        char* newString = 0;
        int previousAllocated = m_Allocated;

        // what's the target type and how much space will we need
        switch(m_Type)
        {
        case SFFSTRType_ANSI:
        {
            m_Allocated = ((((m_Length+1) / SFFString_ALLOC_BLOCK_SIZE) + 1) * SFFString_ALLOC_BLOCK_SIZE);
            newString = new char[m_Allocated];

            memset(newString,0,m_Allocated);

            if(m_szBuffer)
            {
                memcpy(newString,m_szBuffer, std::min<DWORD>(m_Length + m_CharacterWidth,previousAllocated));
            }
        }
        break;

        case SFFSTRType_Unicode16:
        case SFFSTRType_UTF8:
        {
            m_Allocated = ((((m_Length+1) / SFFString_ALLOC_BLOCK_SIZE) + 1) * SFFString_ALLOC_BLOCK_SIZE) * m_CharacterWidth;
            newString = new char[m_Allocated];

            memset(newString,0,m_Allocated);

            if(m_szBuffer)
            {
                memcpy(newString, m_szBuffer, std::min<DWORD>((m_Length + m_CharacterWidth) * 2, previousAllocated));
            }
        }
        break;
        }

        if(m_szBuffer != 0)
            delete [] m_szBuffer;

        m_szBuffer = newString;
    }
}

//======================================================================================================================

USHORT SFFString::getLength() const
{
    return m_Length;
}

//======================================================================================================================

DWORD SFFString::getDataLength() const
{
    return m_Length * m_CharacterWidth;
}

//======================================================================================================================

DWORD SFFString::getCharacterWidth() const
{
    return m_CharacterWidth;
}

//======================================================================================================================

SFFStringType SFFString::getType() const
{
    return m_Type;
}

//======================================================================================================================

char* SFFString::getRawData() const
{
    return m_szBuffer;
}

//======================================================================================================================

DWORD SFFString::getAllocated() const
{
    return m_Allocated;
}

//======================================================================================================================
DWORD SFFString::CRC(char* data)
{
    DWORD length = (DWORD)strlen(data);

    DWORD crc = 0xffffffff;  // starting seed
    for (DWORD i = 0; i < length; i++)
    {
        crc = mCrcTable[data[i] ^ (crc >> 24)] ^ (crc << 8);
    }

    return ~crc;
}


//======================================================================================================================
DWORD SFFString::mCrcTable[256] =
{
    0x0000000,
    0x04C11DB7, 0x09823B6E, 0x0D4326D9, 0x130476DC, 0x17C56B6B,
    0x1A864DB2, 0x1E475005, 0x2608EDB8, 0x22C9F00F, 0x2F8AD6D6,
    0x2B4BCB61, 0x350C9B64, 0x31CD86D3, 0x3C8EA00A, 0x384FBDBD,
    0x4C11DB70, 0x48D0C6C7, 0x4593E01E, 0x4152FDA9, 0x5F15ADAC,
    0x5BD4B01B, 0x569796C2, 0x52568B75, 0x6A1936C8, 0x6ED82B7F,
    0x639B0DA6, 0x675A1011, 0x791D4014, 0x7DDC5DA3, 0x709F7B7A,
    0x745E66CD, 0x9823B6E0, 0x9CE2AB57, 0x91A18D8E, 0x95609039,
    0x8B27C03C, 0x8FE6DD8B, 0x82A5FB52, 0x8664E6E5, 0xBE2B5B58,
    0xBAEA46EF, 0xB7A96036, 0xB3687D81, 0xAD2F2D84, 0xA9EE3033,
    0xA4AD16EA, 0xA06C0B5D, 0xD4326D90, 0xD0F37027, 0xDDB056FE,
    0xD9714B49, 0xC7361B4C, 0xC3F706FB, 0xCEB42022, 0xCA753D95,
    0xF23A8028, 0xF6FB9D9F, 0xFBB8BB46, 0xFF79A6F1, 0xE13EF6F4,
    0xE5FFEB43, 0xE8BCCD9A, 0xEC7DD02D, 0x34867077, 0x30476DC0,
    0x3D044B19, 0x39C556AE, 0x278206AB, 0x23431B1C, 0x2E003DC5,
    0x2AC12072, 0x128E9DCF, 0x164F8078, 0x1B0CA6A1, 0x1FCDBB16,
    0x018AEB13, 0x054BF6A4, 0x0808D07D, 0x0CC9CDCA, 0x7897AB07,
    0x7C56B6B0, 0x71159069, 0x75D48DDE, 0x6B93DDDB, 0x6F52C06C,
    0x6211E6B5, 0x66D0FB02, 0x5E9F46BF, 0x5A5E5B08, 0x571D7DD1,
    0x53DC6066, 0x4D9B3063, 0x495A2DD4, 0x44190B0D, 0x40D816BA,
    0xACA5C697, 0xA864DB20, 0xA527FDF9, 0xA1E6E04E, 0xBFA1B04B,
    0xBB60ADFC, 0xB6238B25, 0xB2E29692, 0x8AAD2B2F, 0x8E6C3698,
    0x832F1041, 0x87EE0DF6, 0x99A95DF3, 0x9D684044, 0x902B669D,
    0x94EA7B2A, 0xE0B41DE7, 0xE4750050, 0xE9362689, 0xEDF73B3E,
    0xF3B06B3B, 0xF771768C, 0xFA325055, 0xFEF34DE2, 0xC6BCF05F,
    0xC27DEDE8, 0xCF3ECB31, 0xCBFFD686, 0xD5B88683, 0xD1799B34,
    0xDC3ABDED, 0xD8FBA05A, 0x690CE0EE, 0x6DCDFD59, 0x608EDB80,
    0x644FC637, 0x7A089632, 0x7EC98B85, 0x738AAD5C, 0x774BB0EB,
    0x4F040D56, 0x4BC510E1, 0x46863638, 0x42472B8F, 0x5C007B8A,
    0x58C1663D, 0x558240E4, 0x51435D53, 0x251D3B9E, 0x21DC2629,
    0x2C9F00F0, 0x285E1D47, 0x36194D42, 0x32D850F5, 0x3F9B762C,
    0x3B5A6B9B, 0x0315D626, 0x07D4CB91, 0x0A97ED48, 0x0E56F0FF,
    0x1011A0FA, 0x14D0BD4D, 0x19939B94, 0x1D528623, 0xF12F560E,
    0xF5EE4BB9, 0xF8AD6D60, 0xFC6C70D7, 0xE22B20D2, 0xE6EA3D65,
    0xEBA91BBC, 0xEF68060B, 0xD727BBB6, 0xD3E6A601, 0xDEA580D8,
    0xDA649D6F, 0xC423CD6A, 0xC0E2D0DD, 0xCDA1F604, 0xC960EBB3,
    0xBD3E8D7E, 0xB9FF90C9, 0xB4BCB610, 0xB07DABA7, 0xAE3AFBA2,
    0xAAFBE615, 0xA7B8C0CC, 0xA379DD7B, 0x9B3660C6, 0x9FF77D71,
    0x92B45BA8, 0x9675461F, 0x8832161A, 0x8CF30BAD, 0x81B02D74,
    0x857130C3, 0x5D8A9099, 0x594B8D2E, 0x5408ABF7, 0x50C9B640,
    0x4E8EE645, 0x4A4FFBF2, 0x470CDD2B, 0x43CDC09C, 0x7B827D21,
    0x7F436096, 0x7200464F, 0x76C15BF8, 0x68860BFD, 0x6C47164A,
    0x61043093, 0x65C52D24, 0x119B4BE9, 0x155A565E, 0x18197087,
    0x1CD86D30, 0x029F3D35, 0x065E2082, 0x0B1D065B, 0x0FDC1BEC,
    0x3793A651, 0x3352BBE6, 0x3E119D3F, 0x3AD08088, 0x2497D08D,
    0x2056CD3A, 0x2D15EBE3, 0x29D4F654, 0xC5A92679, 0xC1683BCE,
    0xCC2B1D17, 0xC8EA00A0, 0xD6AD50A5, 0xD26C4D12, 0xDF2F6BCB,
    0xDBEE767C, 0xE3A1CBC1, 0xE760D676, 0xEA23F0AF, 0xEEE2ED18,
    0xF0A5BD1D, 0xF464A0AA, 0xF9278673, 0xFDE69BC4, 0x89B8FD09,
    0x8D79E0BE, 0x803AC667, 0x84FBDBD0, 0x9ABC8BD5, 0x9E7D9662,
    0x933EB0BB, 0x97FFAD0C, 0xAFB010B1, 0xAB710D06, 0xA6322BDF,
    0xA2F33668, 0xBCB4666D, 0xB8757BDA, 0xB5365D03, 0xB1F740B4,
};




//======================================================================================================================
void SFFString::setLength(USHORT length)
{
    m_Length = length;
    _allocate();
}


//======================================================================================================================
void SFFString::setType(SFFStringType type)
{
    m_Type = type;     // what's the target type and how much space will we need
    switch (m_Type)
    {
    case SFFSTRType_ANSI:
        m_CharacterWidth = 1;
        break;
    case SFFSTRType_Unicode16:
    case SFFSTRType_UTF8:
        m_CharacterWidth = 2;
        break;
    }
}


//======================================================================================================================
DWORD SFFString::getCrc() const
{
    DWORD crc = 0xffffffff;  // starting seed
    for (DWORD i = 0; i < m_Length; i++)
    {
        crc = mCrcTable[m_szBuffer[i] ^ (crc >> 24)] ^ (crc << 8);
    }

    return ~crc;
}