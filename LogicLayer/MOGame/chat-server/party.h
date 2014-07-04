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

#ifndef PARTY_H
#define PARTY_H

#include <string>
#include <vector>

/**
 * A party that contains 1 or more characters to play together
 */
class Party
{
public:
    typedef std::vector<std::string> PartyUsers;

    Party();

    /**
     * Add user to party
     */
    void addUser(const std::string &name,
                 const std::string &inviter = std::string());

    /**
     * Remove user from party
     */
    void removeUser(const std::string &name);

    /**
     * Return number of users in party
     */
    unsigned userCount() const { return mUsers.size(); }

    /**
     * Return the party id
     */
    unsigned getId() const { return mId; }

    const PartyUsers &getUsers() const { return mUsers; }

private:
    PartyUsers mUsers;

    unsigned mId;
};

#endif
