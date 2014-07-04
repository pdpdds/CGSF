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

#ifndef MONSTERMANAGER_H
#define MONSTERMANAGER_H

#include <string>
#include <map>
#include "utils/string.h"
#include "utils/xml.h"


class MonsterClass;

typedef std::map< int, MonsterClass * > MonsterClasses;

class MonsterManager
{
    public:
        MonsterManager()
        {}

        ~MonsterManager()
        { deinitialize(); }

        /**
         * Loads monster reference file.
         */
        void initialize();

        /**
         * Reloads monster reference file.
         */
        void reload();

        /**
         * Destroy monster classes.
         */
        void deinitialize();

        /**
         * Gets the MonsterClass having the given ID.
         */
        MonsterClass *getMonster(int id) const;

        /**
         * Gets the first monster type with a specific name.
         * (comparison is case-insensitive).
         * Returns null when there is no monster with such
         * a name.
         */
        MonsterClass *getMonsterByName(const std::string &name) const;

        const MonsterClasses &getMonsterClasses() const;

        void readMonsterNode(xmlNodePtr node, const std::string &filename);

        void checkStatus();

    private:
        MonsterClasses mMonsterClasses; /**< Monster reference */
        utils::NameMap<MonsterClass*> mMonsterClassesByName;
};

inline const MonsterClasses &MonsterManager::getMonsterClasses() const
{
    return mMonsterClasses;
}

extern MonsterManager *monsterManager;

#endif // MONSTERMANAGER_H
