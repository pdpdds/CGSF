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


#ifndef ABILITYMANAGER_H
#define ABILITYMANAGER_H

#include "utils/string.h"
#include "utils/xml.h"

#include "scripting/script.h"



class AbilityManager
{
public:
    enum TargetMode
    {
        TARGET_BEING,
        TARGET_POINT,
        TARGET_DIRECTION,
    };

    struct AbilityInfo
    {
        AbilityInfo() :
            id(0),
            target(TARGET_BEING)
        {}

        unsigned id;
        std::string name;
        TargetMode target;
        Script::Ref rechargedCallback;
        Script::Ref useCallback;
    };

    AbilityManager()
    { }

    ~AbilityManager()
    { clear(); }

    /**
     * Loads ability reference file.
     */
    void initialize();

    /**
     * Reloads ability reference file.
     */
    void reload();

    /**
     * Gets the abilities Id from a string formatted in this way:
     * "categoryname_skillname"
     */
    unsigned getId(const std::string &abilityName) const;

    const std::string getAbilityName(int id) const;

    AbilityInfo *getAbilityInfo(int id) const;
    AbilityInfo *getAbilityInfo(const std::string &abilityName) const;

    void checkStatus();

    void readAbilityNode(xmlNodePtr skillNode, const std::string &filename);

private:
    /**
     * Clears up the ability maps.
     */
    void clear();

    typedef std::map<unsigned, AbilityInfo*> AbilitiesInfo;
    AbilitiesInfo mAbilitiesInfo;
    typedef utils::NameMap<AbilityInfo*> NamedAbilitiesInfo;
    NamedAbilitiesInfo mNamedAbilitiesInfo;

};

extern AbilityManager *abilityManager;

#endif // ABILITYMANAGER_H
