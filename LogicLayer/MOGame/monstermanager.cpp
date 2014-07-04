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

#include "monstermanager.h"

#include "common/defines.h"

#include "attributemanager.h"
#include "itemmanager.h"
#include "monster.h"
#include "utils/logger.h"

#define MAX_MUTATION 99
#define DEFAULT_MONSTER_SIZE 16
#define DEFAULT_MONSTER_SPEED 4.0f

void MonsterManager::reload()
{
    deinitialize();
    initialize();
}

void MonsterManager::initialize()
{

}

void MonsterManager::deinitialize()
{
    for (MonsterClasses::iterator i = mMonsterClasses.begin(),
         i_end = mMonsterClasses.end(); i != i_end; ++i)
    {
        delete i->second;
    }
    mMonsterClasses.clear();
    mMonsterClassesByName.clear();
}

MonsterClass *MonsterManager::getMonsterByName(const std::string &name) const
{
    return mMonsterClassesByName.value(name);
}

MonsterClass *MonsterManager::getMonster(int id) const
{
    MonsterClasses::const_iterator i = mMonsterClasses.find(id);
    return i != mMonsterClasses.end() ? i->second : 0;
}

/**
 * Read a <monster> element from settings.
 * Used by SettingsManager.
 */
void MonsterManager::readMonsterNode(xmlNodePtr node, const std::string &filename)
{
    if (!xmlStrEqual(node->name, BAD_CAST "monster"))
        return;

    int monsterId = XML::getProperty(node, "id", 0);
    std::string name = XML::getProperty(node, "name", std::string());

    if (monsterId < 1)
    {
        LOG_WARN("Monster Manager: Ignoring monster ("
                 << name << ") without Id in "
                 << filename << "! It has been ignored.");
        return;
    }

    MonsterClasses::iterator i = mMonsterClasses.find(monsterId);
    if (i != mMonsterClasses.end())
    {
        LOG_WARN("Monster Manager: Ignoring duplicate definition of "
                 "monster '" << monsterId << "'!");
        return;
    }

    MonsterClass *monster = new MonsterClass(monsterId);
    mMonsterClasses[monsterId] = monster;

    if (!name.empty())
    {
        monster->setName(name);

        if (mMonsterClassesByName.contains(name))
            LOG_WARN("Monster Manager: Name not unique for monster "
                     << monsterId);
        else
            mMonsterClassesByName.insert(name, monster);
    }

    MonsterDrops drops;

    for_each_xml_child_node(subnode, node)
    {
        if (xmlStrEqual(subnode->name, BAD_CAST "drop"))
        {
            MonsterDrop drop;
            std::string item = XML::getProperty(subnode, "item",
                                                std::string());
            ItemClass *itemClass;
            if (utils::isNumeric(item))
                itemClass = itemManager->getItem(utils::stringToInt(item));
            else
                itemClass = itemManager->getItemByName(item);

            if (!itemClass)
            {
                LOG_WARN("Monster Manager: Invalid item name \"" << item
                         << "\"");
                break;
            }

            drop.item = itemClass;
            drop.probability = XML::getFloatProperty(subnode, "percent",
                                                     0.0) * 100 + 0.5;

            if (drop.probability)
                drops.push_back(drop);
        }
        else if (xmlStrEqual(subnode->name, BAD_CAST "attributes"))
        {
            monster->setSize(XML::getProperty(subnode, "size", -1));
            monster->setMutation(XML::getProperty(subnode, "mutation", 0));
            std::string genderString = XML::getProperty(subnode, "gender",
                                                        std::string());
            monster->setGender(getGender(genderString));

            // Checking attributes for completeness and plausibility
            if (monster->getMutation() > MAX_MUTATION)
            {
                LOG_WARN(filename
                         << ": Mutation of monster Id:" << monsterId
                         << " more than " << MAX_MUTATION
                         << "%. Defaulted to 0.");
                monster->setMutation(0);
            }

            if (monster->getSize() == -1)
            {
                LOG_WARN(filename
                         << ": No size set for monster Id:" << monsterId << ". "
                         << "Defaulted to " << DEFAULT_MONSTER_SIZE
                         << " pixels.");
                monster->setSize(DEFAULT_MONSTER_SIZE);
            }
        }
        else if (xmlStrEqual(subnode->name, BAD_CAST "attribute"))
        {
            std::string attributeIdString = XML::getProperty(subnode, "id",
                                                             std::string());
            AttributeInfo *info = nullptr;
            if (utils::isNumeric(attributeIdString))
            {
                const int attributeId = utils::stringToInt(attributeIdString);
                info = attributeManager->getAttributeInfo(attributeId);
            }
            else
            {
                info = attributeManager->getAttributeInfo(attributeIdString);
            }

            if (!info)
            {
                LOG_WARN(filename
                         << ": Invalid attribute id " << attributeIdString
                         << " for monster Id: " << monsterId
                         << ". Skipping!");
                continue;
            }

            const double value = XML::getFloatProperty(subnode, "value", 0.0);

            monster->setAttribute(info, value);
        }
        else if (xmlStrEqual(subnode->name, BAD_CAST "ability"))
        {
            const std::string idText = XML::getProperty(subnode, "id",
                                                        std::string());
            AbilityManager::AbilityInfo *info = 0;
            if (utils::isNumeric(idText))
            {
                const int abilityId = utils::stringToInt(idText);
                info = abilityManager->getAbilityInfo(abilityId);
            }
            else
            {
                info = abilityManager->getAbilityInfo(idText);
            }

            if (!info)
            {
                LOG_WARN(filename
                         << ": Invalid ability id " << idText
                         << " for monster id: " << monsterId
                         << " Skipping!");
                continue;
            }

            monster->addAbility(info);
        }
    }

    monster->setDrops(drops);
}

/**
 * Check the status of recently loaded configuration.
 */
void MonsterManager::checkStatus()
{
    LOG_INFO("Loaded " << mMonsterClasses.size() << " monsters");
}
