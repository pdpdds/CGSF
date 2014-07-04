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

#ifndef MESSAGEOUT_H
#define MESSAGEOUT_H

#include "common/manaserv_protocol.h"

#include <iosfwd>

/**
 * Used for building an outgoing message.
 */
class MessageOut
{
    public:
        /**
         * Constructor.
         *
         * @param id the message ID
         */
        MessageOut(int id);

        ~MessageOut();

        /**
         * Writes an 8-bit integer to the message.
         */
        void writeInt8(int value);

        /**
         * Writes an 16-bit integer to the message.
         */
        void writeInt16(int value);

        /**
         * Writes an 32-bit integer to the message.
         */
        void writeInt32(int value);

        /**
         * Writes a double. HACKY and should *not* be used for client
         * communication!
         */
        void writeDouble(double value);

        /**
         * Writes a string. If a fixed length is not given (-1), it is stored
         * as a short at the start of the string.
         */
        void writeString(const std::string &string, int length = -1);

        /**
         * Returns the content of the message.
         */
        char *getData() const { return mData; }

        /**
         * Returns the length of the data.
         */
        unsigned getLength() const { return mPos; }

        /**
         * Sets whether the debug mode is enabled. In debug mode, the internal
         * data of the message is annotated so that the message contents can
         * be printed.
         *
         * Debug mode is disabled by default.
         */
        static void setDebugModeEnabled(bool enabled);

    private:
        /**
         * Ensures the capacity of the data buffer is large enough to hold the
         * given amount of bytes.
         */
        void expand(size_t size);

        void writeValueType(ManaServ::ValueType type);

        char *mData;                /**< Data building up. */
        unsigned mPos;              /**< Position in the data. */
        unsigned mDataSize;         /**< Allocated datasize. */
        bool mDebugMode;            /**< Include debugging information. */

        /**
         * Streams message ID and length to the given output stream.
         */
        friend std::ostream& operator <<(std::ostream &os,
                                         const MessageOut &msg);
};

#endif // MESSAGEOUT_H
