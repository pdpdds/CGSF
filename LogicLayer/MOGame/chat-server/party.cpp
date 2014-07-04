/*
 *  The Mana Server
 *  Copyright (C) 2008-2010  The Mana World Development Team
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

#include "party.h"

#include "chatclient.h"
#include "chathandler.h"

#include "common/manaserv_protocol.h"

#include "net/messageout.h"

#include <algorithm>

Party::Party()
{
    static int id = 0;
    id++;
    mId = id;
}

void Party::addUser(const std::string &name, const std::string &inviter)
{
    mUsers.push_back(name);

    for (size_t i = 0; i < userCount(); ++i)
    {
        MessageOut out(ManaServ::CPMSG_PARTY_NEW_MEMBER);
        out.writeString(name);
        out.writeString(inviter);
        chatHandler->getClient(mUsers[i])->send(out);
    }
}

void Party::removeUser(const std::string &name)
{
    PartyUsers::iterator itr = std::find(mUsers.begin(), mUsers.end(), name);
    if (itr != mUsers.end())
    {
        mUsers.erase(itr);
    }
}
