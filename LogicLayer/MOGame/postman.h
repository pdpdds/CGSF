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

#ifndef POSTMAN_H
#define POSTMAN_H

#include <map>
#include <string>

#include "charactercomponent.h"
#include "entity.h"

class Script;

struct PostCallback
{
    void (*handler)(Entity *,
                    const std::string &sender,
                    const std::string &letter,
                    Script *);

    Script *script;
};

class PostMan
{
public:
    Entity *getCharacter(int id) const
    {
        std::map<int, Entity*>::const_iterator itr = mCharacters.find(id);
        if (itr != mCharacters.end())
            return itr->second;
        return 0;
    }

    void addCharacter(Entity *player)
    {
        int dataBaseId = player->getComponent<CharacterComponent>()
                ->getDatabaseID();
        std::map<int, Entity*>::iterator itr = mCharacters.find(dataBaseId);
        if (itr == mCharacters.end())
        {
            mCharacters.insert(std::pair<int, Entity*>(dataBaseId, player));
        }
    }

    void getPost(Entity *player, PostCallback &f)
    {
        mCallbacks.insert(std::pair<Entity*, PostCallback>(player, f));
        accountHandler->getPost(player);
    }

    void gotPost(Entity *player, std::string sender, std::string letter)
    {
        std::map<Entity*, PostCallback>::iterator itr = mCallbacks.find(player);
        if (itr != mCallbacks.end())
        {
            itr->second.handler(player, sender, letter, itr->second.script);
        }
    }

private:
    std::map<int, Entity*> mCharacters;
    std::map<Entity*, PostCallback> mCallbacks;
};

extern PostMan *postMan;

#endif
