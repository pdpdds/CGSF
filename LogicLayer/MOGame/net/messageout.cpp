/*
 *  The Mana Server
 *  Copyright (C) 2004-2010  The Mana World Development Team
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

#include "net/messageout.h"
#include "net/messagein.h"

#include <cstring>
#include <iomanip>
#include <iostream>
#ifndef USE_NATIVE_DOUBLE
#include <limits>
#include <sstream>
#endif
#include <stdint.h>
#include <string>
#include <enet/enet.h>

/** Initial amount of bytes allocated for the messageout data buffer. */
const unsigned INITIAL_DATA_CAPACITY = 16;

/** Factor by which the messageout data buffer is increased when too small. */
const unsigned CAPACITY_GROW_FACTOR = 2;

static bool debugModeEnabled = false;

MessageOut::MessageOut(int id):
    mPos(0),
    mDebugMode(false)
{
    mData = (char*) malloc(INITIAL_DATA_CAPACITY);
    mDataSize = INITIAL_DATA_CAPACITY;

    if (debugModeEnabled)
        id |= ManaServ::XXMSG_DEBUG_FLAG;

    writeInt16(id);
    mDebugMode = debugModeEnabled;
}

MessageOut::~MessageOut()
{
    free(mData);
}

void MessageOut::expand(size_t bytes)
{
    if (bytes > mDataSize)
    {
        do
        {
            mDataSize *= CAPACITY_GROW_FACTOR;
        }
        while (bytes > mDataSize);

        mData = (char*) realloc(mData, mDataSize);
    }
}

void MessageOut::writeInt8(int value)
{
    if (mDebugMode)
        writeValueType(ManaServ::Int8);

    expand(mPos + 1);
    mData[mPos] = value;
    mPos += 1;
}

void MessageOut::writeInt16(int value)
{
    if (mDebugMode)
        writeValueType(ManaServ::Int16);

    expand(mPos + 2);
    uint16_t t = ENET_HOST_TO_NET_16(value);
    memcpy(mData + mPos, &t, 2);
    mPos += 2;
}

void MessageOut::writeInt32(int value)
{
    if (mDebugMode)
        writeValueType(ManaServ::Int32);

    expand(mPos + 4);
    uint32_t t = ENET_HOST_TO_NET_32(value);
    memcpy(mData + mPos, &t, 4);
    mPos += 4;
}

void MessageOut::writeDouble(double value)
{
    if (mDebugMode)
        writeValueType(ManaServ::Double);

#ifdef USE_NATIVE_DOUBLE
    expand(mPos + sizeof(double));
    memcpy(mData + mPos, &value, sizeof(double));
    mPos += sizeof(double);
#else
// Rather inefficient, but I don't have a lot of time.
// If anyone wants to implement a custom double you are more than welcome to.
    std::ostringstream o;
    // Highest precision for double
    o.precision(std::numeric_limits< double >::digits10);
    o << value;
    std::string str = o.str();
    writeInt8(str.size());
    writeString(str, str.size());
#endif
}

void MessageOut::writeString(const std::string &string, int length)
{
    if (mDebugMode)
    {
        writeValueType(ManaServ::String);
        writeInt16(length);
    }

    int stringLength = string.length();
    if (length < 0)
    {
        // Write the length at the start if not fixed
        writeInt16(stringLength);
        length = stringLength;
    }
    else if (length < stringLength)
    {
        // Make sure the length of the string is no longer than specified
        stringLength = length;
    }
    expand(mPos + length);

    // Write the actual string
    memcpy(mData + mPos, string.data(), stringLength);

    if (length > stringLength)
    {
        // Pad remaining space with zeros
        memset(mData + mPos + stringLength, '\0', length - stringLength);
    }
    mPos += length;
}

void MessageOut::writeValueType(ManaServ::ValueType type)
{
    expand(mPos + 1);
    mData[mPos] = type;
    mPos += 1;
}

std::ostream&
operator <<(std::ostream &os, const MessageOut &msg)
{
    if (msg.getLength() >= 2)
    {
        MessageIn m(msg.mData, msg.mPos);
        os << m;
    }
    else
    {
        os << "Unknown"
           << std::dec << " (" << msg.getLength() << " B)";
    }
    return os;
}

void MessageOut::setDebugModeEnabled(bool enabled)
{
    debugModeEnabled = enabled;
}
