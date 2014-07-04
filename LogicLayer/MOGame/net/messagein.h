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

#ifndef MESSAGEIN_H
#define MESSAGEIN_H

#include "common/manaserv_protocol.h"

#include <iosfwd>

/**
 * Used for parsing an incoming message.
 */
class MessageIn
{
    public:
        /**
         * Constructor.
         *
         * @param data   the message data
         * @param length the length of the data
         */
        MessageIn(const char *data, unsigned short length);

        /**
         * Returns the message ID.
         */
        int getId() const { return mId; }

        /**
         * Returns the total length of this message.
         */
        int getLength() const { return mLength; }

        int readInt8();             /**< Reads a byte. */
        int readInt16();            /**< Reads a short. */
        int readInt32();            /**< Reads a long. */

        /**
         * Reads a double. HACKY and should *not* be used for client
         * communication!
         */
        double readDouble();

        /**
         * Reads a string. If a length is not given (-1), it is assumed
         * that the length of the string is stored in a short at the
         * start of the string.
         */
        std::string readString(int length = -1);

        /**
         * Returns the length of unread data.
         */
        int getUnreadLength() const { return mLength - mPos; }

    private:
        bool readValueType(ManaServ::ValueType type);

        const char *mData;            /**< Packet data */
        unsigned short mLength;       /**< Length of data in bytes */
        unsigned short mId;           /**< The message ID. */
        bool mDebugMode;              /**< Includes debugging information. */

        /**
         * Actual position in the packet. From 0 to packet->length. A value
         * bigger than packet->length means EOP was reached when reading it.
         */
        unsigned short mPos;

        /**
         * Streams message ID and length to the given output stream.
         *
         * When the message includes debugging information, prints out
         * the message contents instead of the length.
         */
        friend std::ostream& operator <<(std::ostream &os,
                                         const MessageIn &msg);
};

#endif
