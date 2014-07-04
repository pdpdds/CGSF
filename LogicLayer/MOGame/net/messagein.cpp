/*
 *  The Mana Server
 *  Copyright (C) 2004-2010  The Mana World Development Team
 *  Copyright (C) 2010-2012  The Mana Developers
 *
 *  This file is part of The Mana Server.
 *
 *  The Mana Server is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana Server is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana Server.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cstring>
#include <iomanip>
#include <iostream>
#include <string>
#include <enet/enet.h>
#ifndef USE_NATIVE_DOUBLE
#include <sstream>
#endif
#include <stdint.h>

#include "net/messagein.h"
#include "utils/logger.h"

// Not enabled by default since this will cause assertions on message errors,
// which may also originate from the client.
//#define DEBUG_NETWORK

#ifdef DEBUG_NETWORK
#include <cassert>
#define ASSERT_IF(x) assert(x); if (x)
#else
#define ASSERT_IF(x) if (x)
#endif

MessageIn::MessageIn(const char *data, unsigned short length):
    mData(data),
    mLength(length),
    mDebugMode(false),
    mPos(0)
{
    // Read the message ID
    mId = readInt16();

    // Read and clear the debug flag
    mDebugMode = mId & ManaServ::XXMSG_DEBUG_FLAG;
    mId &= ~ManaServ::XXMSG_DEBUG_FLAG;
}

int MessageIn::readInt8()
{
    int value = -1;

    if (!readValueType(ManaServ::Int8))
        return value;

    ASSERT_IF (mPos < mLength)
    {
        value = (unsigned char) mData[mPos];
    }
    else
    {
        LOG_DEBUG("Unable to read 1 byte in " << mId << "!");
    }

    mPos += 1;
    return value;
}

int MessageIn::readInt16()
{
    int value = -1;

    if (!readValueType(ManaServ::Int16))
        return value;

    ASSERT_IF (mPos + 2 <= mLength)
    {
        uint16_t t;
        memcpy(&t, mData + mPos, 2);
        value = (short) ENET_NET_TO_HOST_16(t);
    }
    else
    {
        LOG_DEBUG("Unable to read 2 bytes in " << mId << "!");
    }

    mPos += 2;
    return value;
}

int MessageIn::readInt32()
{
    int value = -1;

    if (!readValueType(ManaServ::Int32))
        return value;

    ASSERT_IF (mPos + 4 <= mLength)
    {
        uint32_t t;
        memcpy(&t, mData + mPos, 4);
        value = ENET_NET_TO_HOST_32(t);
    }
    else
    {
        LOG_DEBUG("Unable to read 4 bytes in " << mId << "!");
    }

    mPos += 4;
    return value;
}

double MessageIn::readDouble()
{
    double value = -1;

    if (!readValueType(ManaServ::Double))
        return value;

#ifdef USE_NATIVE_DOUBLE
    if (mPos + sizeof(double) <= mLength)
        memcpy(&value, mData + mPos, sizeof(double));
    mPos += sizeof(double);
#else
    int length = readInt8();
    std::istringstream i (readString(length));
    i >> value;
#endif
    return value;
}

std::string MessageIn::readString(int length)
{
    if (!readValueType(ManaServ::String))
        return std::string();

    if (mDebugMode)
    {
        int fixedLength = readInt16();
#ifdef DEBUG_NETWORK
        assert(fixedLength == length);
#endif
        if (fixedLength != length)
        {
            LOG_DEBUG("Expected string of length " << length <<
                      " but received length " << fixedLength);
            mPos = mLength + 1;
            return std::string();
        }
    }

    // Get string length
    if (length < 0)
    {
        length = readInt16();
    }

    // Make sure the string isn't erroneous
    if (length < 0 || mPos + length > mLength)
    {
        mPos = mLength + 1;
        return std::string();
    }

    // Read the string
    const char *stringBeg = mData + mPos;
    const char *stringEnd = (const char *)memchr(stringBeg, '\0', length);
    std::string readString(stringBeg,
                           stringEnd ? stringEnd - stringBeg : length);
    mPos += length;

    return readString;
}

bool MessageIn::readValueType(ManaServ::ValueType type)
{
    if (!mDebugMode) // Verification not possible
        return true;

    ASSERT_IF (mPos < mLength)
    {
        int t = (unsigned char) mData[mPos];
        ++mPos;

        ASSERT_IF (t == type)
        {
            return true;
        }
        else
        {
            LOG_DEBUG("Attempt to read " << type << " but got " << t);
        }
    }
    else
    {
        LOG_DEBUG("Attempt to read " << type << " but no more data available");
    }

    return false;
}

std::ostream&
operator <<(std::ostream &os, const MessageIn &msg)
{
    os << std::setw(6) << std::hex << std::showbase << std::internal
       << std::setfill('0') << msg.getId() << std::dec;

    if (!msg.mDebugMode)
    {
        os << " (" << msg.getLength() << " B)";
    }
    else
    {
        os << " { ";

        MessageIn m(msg.mData, msg.mLength);

        while (m.getUnreadLength() > 0)
        {
            if (m.mPos > 3)
                os << ", ";

            unsigned char valueType = m.mData[m.mPos];
            switch (valueType)
            {
            case ManaServ::Int8:
                os << "B " << m.readInt8();
                break;
            case ManaServ::Int16:
                os << "W " << m.readInt16();
                break;
            case ManaServ::Int32:
                os << "D " << m.readInt32();
                break;
            case ManaServ::String: {
                // Peak ahead at a possible fixed length
                unsigned short pos = m.mPos;
                m.mPos++;
                int length = m.readInt16();
                m.mPos = pos;

                if (length < 0)
                    os << "S " << m.readString();
                else
                    os << "S[" << length << "] " << m.readString(length);
                break;
            }
            case ManaServ::Double:
                os << "d " << m.readDouble();
                break;
            default:
                os << "??? }";
                return os; // Stop after error
            }
        }

        os << " }";
    }

    return os;
}
