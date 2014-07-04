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

#include "statusmanager.h"

#include "common/resourcemanager.h"
#include "statuseffect.h"
#include "utils/logger.h"
#include "utils/xml.h"

#include <map>
#include <set>
#include <sstream>

typedef std::map< int, StatusEffect * > StatusEffectsMap;
static StatusEffectsMap statusEffects;
static utils::NameMap<StatusEffect*> statusEffectsByName;

void StatusManager::initialize()
{

}

void StatusManager::reload()
{
    deinitialize();
}

void StatusManager::deinitialize()
{
    for (StatusEffectsMap::iterator i = statusEffects.begin(),
           i_end = statusEffects.end(); i != i_end; ++i)
    {
        delete i->second;
    }
    statusEffects.clear();
    statusEffectsByName.clear();
}

StatusEffect *StatusManager::getStatus(int statusId)
{
    StatusEffectsMap::const_iterator i = statusEffects.find(statusId);
    return i != statusEffects.end() ? i->second : 0;
}

StatusEffect *StatusManager::getStatusByName(const std::string &name)
{
    return statusEffectsByName.value(name);
}

/**
 * Read a <attribute> element from settings.
 * Used by SettingsManager.
 */
void StatusManager::readStatusNode(xmlNodePtr node, const std::string &filename)
{
    const int id = XML::getProperty(node, "id", 0);
    if (id < 1)
    {
        LOG_WARN("Status Manager: The status ID: " << id << " in "
                 << filename
                 << " is invalid and will be ignored.");
        return;
    }

    StatusEffect *statusEffect = new StatusEffect(id);

    const std::string name = XML::getProperty(node, "name",
                                              std::string());
    if (!name.empty())
    {
        if (statusEffectsByName.contains(name))
        {
            LOG_WARN("StatusManager: name not unique for status effect "
                     << id);
        }
        else
        {
            statusEffectsByName.insert(name, statusEffect);
        }
    }

    //TODO: Get these modifiers
/*
    modifiers.setAttributeValue(BASE_ATTR_PHY_ATK_MIN,      XML::getProperty(node, "attack-min",      0));
    modifiers.setAttributeValue(BASE_ATTR_PHY_ATK_DELTA,      XML::getProperty(node, "attack-delta",      0));
    modifiers.setAttributeValue(BASE_ATTR_HP,      XML::getProperty(node, "hp",      0));
    modifiers.setAttributeValue(BASE_ATTR_PHY_RES, XML::getProperty(node, "defense", 0));
    modifiers.setAttributeValue(CHAR_ATTR_STRENGTH,     XML::getProperty(node, "strength",     0));
    modifiers.setAttributeValue(CHAR_ATTR_AGILITY,      XML::getProperty(node, "agility",      0));
    modifiers.setAttributeValue(CHAR_ATTR_DEXTERITY,    XML::getProperty(node, "dexterity",    0));
    modifiers.setAttributeValue(CHAR_ATTR_VITALITY,     XML::getProperty(node, "vitality",     0));
    modifiers.setAttributeValue(CHAR_ATTR_INTELLIGENCE, XML::getProperty(node, "intelligence", 0));
    modifiers.setAttributeValue(CHAR_ATTR_WILLPOWER,    XML::getProperty(node, "willpower",    0));
*/

    statusEffects[id] = statusEffect;

}

/**
 * Check the status of recently loaded configuration.
 */
void StatusManager::checkStatus()
{
    LOG_INFO("Loaded " << statusEffects.size() << " status effects");
}

