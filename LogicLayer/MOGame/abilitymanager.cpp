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

#include "abilitymanager.h"

#include "utils/xml.h"
#include "utils/logger.h"

static AbilityManager::TargetMode getTargetByString(const std::string &str)
{
    std::string strLower = utils::toLower(str);
    if (strLower == "being")
        return AbilityManager::TARGET_BEING;
    else if (strLower == "point")
        return AbilityManager::TARGET_POINT;
    else if (strLower == "direction")
        return AbilityManager::TARGET_DIRECTION;

    LOG_WARN("Unknown targetmode " << str << " assuming being.");
    return AbilityManager::TARGET_BEING;
}

/**
 * Check the status of recently loaded configuration.
 */
void AbilityManager::checkStatus()
{
    LOG_INFO("Loaded " << mAbilitiesInfo.size() << " abilities");
}

void AbilityManager::readAbilityNode(xmlNodePtr abilityNode,
                                     const std::string &filename)
{
    std::string name = utils::toLower(
                XML::getProperty(abilityNode, "name", std::string()));
    int id = XML::getProperty(abilityNode, "id", 0);

    if (id <= 0 || name.empty())
    {
        LOG_WARN("Invalid ability (empty name or id <= 0) in " << filename);
        return;
    }

    AbilitiesInfo::iterator it = mAbilitiesInfo.find(id);
    if (it != mAbilitiesInfo.end())
    {
        LOG_WARN("AbilityManager: The same id: " << id
                 << " is given for ability names: " << it->first
                 << " and " << name);
        LOG_WARN("The ability reference: " << id
                 << ": '" << name << "' will be ignored.");
        return;
    }

    AbilityInfo *newInfo = new AbilityManager::AbilityInfo;
    newInfo->name = name;
    newInfo->id = id;

    newInfo->target = getTargetByString(XML::getProperty(abilityNode, "target",
                                                         std::string()));

    mAbilitiesInfo[newInfo->id] = newInfo;

    mNamedAbilitiesInfo[name] = newInfo;
}

void AbilityManager::initialize()
{
    clear();
}

void AbilityManager::reload()
{
    clear();
}

void AbilityManager::clear()
{
    for (AbilitiesInfo::iterator it = mAbilitiesInfo.begin(),
         it_end = mAbilitiesInfo.end(); it != it_end; ++it)
    {
        delete it->second;
    }
    mAbilitiesInfo.clear();
    mNamedAbilitiesInfo.clear();
}

unsigned AbilityManager::getId(const std::string &abilityName) const
{
    if (mNamedAbilitiesInfo.contains(abilityName))
        return mNamedAbilitiesInfo.value(abilityName)->id;
    else
        return 0;
}

const std::string AbilityManager::getAbilityName(int id) const
{
    AbilitiesInfo::const_iterator it = mAbilitiesInfo.find(id);
    return it != mAbilitiesInfo.end() ? it->second->name : "";
}

AbilityManager::AbilityInfo *AbilityManager::getAbilityInfo(int id) const
{
    AbilitiesInfo::const_iterator it = mAbilitiesInfo.find(id);
    return it != mAbilitiesInfo.end() ? it->second : 0;
}

AbilityManager::AbilityInfo *AbilityManager::getAbilityInfo(
        const std::string &abilityName) const
{
    if (mNamedAbilitiesInfo.contains(abilityName))
        return mNamedAbilitiesInfo.value(abilityName);
    else
        return 0;
}
