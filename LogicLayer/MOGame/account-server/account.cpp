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

#include <cassert>

#include "account-server/account.h"

Account::~Account()
{
    for (Characters::iterator i = mCharacters.begin(),
         i_end = mCharacters.end(); i != i_end; ++i)
    {
        delete (*i).second;
    }
}

bool Account::isSlotEmpty(unsigned slot) const
{
    return mCharacters.find(slot) == mCharacters.end();
}

void Account::setCharacters(const Characters &characters)
{
    mCharacters = characters;
}

void Account::addCharacter(CharacterData *character)
{
    unsigned slot = (unsigned) character->getCharacterSlot();
    assert(isSlotEmpty(slot));

    mCharacters[slot] = character;
}

void Account::delCharacter(unsigned slot)
{
    for (Characters::iterator iter = mCharacters.begin(),
             iter_end = mCharacters.end(); iter != iter_end; ++iter)
    {
        if ((*iter).second->getCharacterSlot() == slot)
        {
            delete (*iter).second;
            (*iter).second = 0;
            mCharacters.erase(iter);
        }
    }
}

void Account::setID(int id)
{
    assert(mID < 0);
    mID = id;
}

void Account::setRegistrationDate(time_t time)
{
    mRegistrationDate = time;
}

void Account::setLastLogin(time_t time)
{
    mLastLogin = time;
}
