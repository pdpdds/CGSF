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

#ifndef TIMEOUT_H
#define TIMEOUT_H

/**
 * @brief A timeout is used to count down to a point of time in the future.
 *
 * This class is actually a passive time keeper. It does not physically count,
 * but it stores a reference time against which the current time is compared.
 *
 * The timeout works in terms of server ticks, which take 100 ms.
 */
class Timeout
{
    public:
        /**
         * @brief Constructs a timeout.
         *
         * By default, the timeout has expired when the server was started.
         */
        Timeout()
            : mReference(0)
        {}

        /**
         * Sets the timeout a given amount of \a ticks in the future.
         */
        void set(int ticks);

        /**
         * Sets the timeout a given amount of \a ticks in the future, unless
         * the timeout is already set to a higher value.
         */
        void setSoft(int ticks);

        /**
         * Returns the number of ticks remaining to the next timeout. Negative
         * when the timeout has already happened.
         */
        int remaining() const;

        /**
         * Returns whether the timeout has expired.
         */
        bool expired() const { return remaining() <= 0; }

        /**
         * Returns whether the timeout was reached in the current tick.
         */
        bool justFinished() const { return remaining() == 0; }

    private:
        int mReference;
};

#endif // TIMEOUT_H
