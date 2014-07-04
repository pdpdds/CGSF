/*
 *  The Mana Server
 *  Copyright (C) 2007-2010  The Mana World Development Team
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

#include "utils/tokencollector.h"

/* We are optimistic and we assume that clients connect as soon as possible.
   It means that pending data are mainly outdated ones, except for the newer
   ones. So we search the lists backward (from newer to older) for already
   inserted data. On the contrary, data are removed when there was a network
   failure, especially a timeout. So we search the lists forward when removing
   data, in order to start from the older ones. */

void TokenCollectorBase::insertClient(const std::string &token, intptr_t data)
{
    for (std::list<Item>::reverse_iterator it = mPendingConnects.rbegin(),
         it_end = mPendingConnects.rend(); it != it_end; ++it)
    {
        if (it->token == token)
        {
            foundMatch(data, it->data);
            mPendingConnects.erase(--it.base());
            return;
        }
    }

    time_t current = time(nullptr);

    Item item;
    item.token = token;
    item.data = data;
    item.timeStamp = current;
    mPendingClients.push_back(item);

    removeOutdated(current);
}

void TokenCollectorBase::insertConnect(const std::string &token, intptr_t data)
{
    for (std::list<Item>::reverse_iterator it = mPendingClients.rbegin(),
         it_end = mPendingClients.rend(); it != it_end; ++it)
    {
        if (it->token == token)
        {
            foundMatch(it->data, data);
            mPendingClients.erase(--it.base());
            return;
        }
    }

    time_t current = time(nullptr);

    Item item;
    item.token = token;
    item.data = data;
    item.timeStamp = current;
    mPendingConnects.push_back(item);

    removeOutdated(current);
}

void TokenCollectorBase::removeClient(intptr_t data)
{
    for (std::list<Item>::iterator it = mPendingClients.begin(),
         it_end = mPendingClients.end(); it != it_end; ++it)
    {
        if (it->data == data)
        {
            mPendingClients.erase(it);
            return;
        }
    }
}

void TokenCollectorBase::removeOutdated(time_t current)
{
    // Timeout happens after 30 seconds. Much longer may actually pass, though.
    time_t threshold = current - 30;
    if (threshold < mLastCheck) return;

    std::list<Item>::iterator it;

    it = mPendingConnects.begin();
    while (it != mPendingConnects.end() && it->timeStamp < threshold)
    {
        removedConnect(it->data);
        it = mPendingConnects.erase(it);
    }

    it = mPendingClients.begin();
    while (it != mPendingClients.end() && it->timeStamp < threshold)
    {
        removedClient(it->data);
        it = mPendingClients.erase(it);
    }

    mLastCheck = current;
}

TokenCollectorBase::TokenCollectorBase():
    mLastCheck(time(nullptr))
{
}

TokenCollectorBase::~TokenCollectorBase()
{
    // Not declared inline, as the list destructors are not trivial.
}
