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

#include "attributemanager.h"

#include "common/defines.h"
#include "attributeinfo.h"
#include "utils/string.h"
#include "utils/logger.h"

void AttributeManager::initialize()
{
}

void AttributeManager::reload()
{
    deinitialize();
    initialize();
}

void AttributeManager::deinitialize()
{
    mTagMap.clear();

    mAttributeNameMap.clear();
    for (auto &it : mAttributeMap)
        delete it.second;
    mAttributeMap.clear();

    for (unsigned i = 0; i < MaxScope; ++i)
        mAttributeScopes[i].clear();
}

AttributeInfo *AttributeManager::getAttributeInfo(
        int id) const
{
    auto ret = mAttributeMap.find(id);
    if (ret == mAttributeMap.end())
        return 0;
    return ret->second;
}

AttributeInfo *AttributeManager::getAttributeInfo(
        const std::string &name) const
{
    if (mAttributeNameMap.contains(name))
        return mAttributeNameMap.value(name);
    return 0;
}

const std::set<AttributeInfo *>
&AttributeManager::getAttributeScope(ScopeType type) const
{
    return mAttributeScopes[type];
}

ModifierLocation AttributeManager::getLocation(const std::string &tag) const
{
    if (mTagMap.find(tag) != mTagMap.end())
        return mTagMap.at(tag);
    else
        return ModifierLocation(0, 0);
}

const std::string *AttributeManager::getTag(const ModifierLocation &location) const
{
    for (auto &it : mTagMap)
    {
        if (it.second == location)
            return &it.first;
    }
    return 0;
}

/**
 * Read a <attribute> element from settings.
 * Used by SettingsManager.
 */
void AttributeManager::readAttributeNode(xmlNodePtr attributeNode)
{
    int id = XML::getProperty(attributeNode, "id", 0);

    if (id <= 0)
    {
        LOG_WARN("Attribute manager: attribute '" << id
                 << "' has an invalid id and will be ignored.");
        return;
    }

    std::string name = XML::getProperty(attributeNode, "name", std::string());
    if (name.empty())
    {
        LOG_WARN("Attribute manager: attribute '" << id
                 << "' does not have a name! Skipping...");
        return;
    }

    AttributeInfo *attribute = new AttributeInfo(id, name);

    attribute->persistent = XML::getBoolProperty(attributeNode, "persistent",
                                                 false);
    attribute->modifiers = std::vector<AttributeModifier>();
    attribute->minimum = XML::getFloatProperty(attributeNode, "minimum",
                                           std::numeric_limits<double>::lowest());
    attribute->maximum = XML::getFloatProperty(attributeNode, "maximum",
                                           std::numeric_limits<double>::max());
    attribute->modifiable = XML::getBoolProperty(attributeNode, "modifiable",
                                                 false);

    const std::string scope = utils::toUpper(
                XML::getProperty(attributeNode, "scope", std::string()));

    bool hasScope = false;

    if (scope.find("CHARACTER") != std::string::npos)
    {
        mAttributeScopes[CharacterScope].insert(attribute);
        LOG_DEBUG("Attribute manager: attribute '" << id
                  << "' added to default character scope.");
        hasScope = true;
    }
    if (scope.find("MONSTER") != std::string::npos)
    {
        mAttributeScopes[MonsterScope].insert(attribute);
        LOG_DEBUG("Attribute manager: attribute '" << id
                  << "' added to default monster scope.");
        hasScope = true;
    }
    if (scope == "BEING")
    {
        mAttributeScopes[BeingScope].insert(attribute);
        LOG_DEBUG("Attribute manager: attribute '" << id
                  << "' added to default being scope.");
        hasScope = true;
    }

    if (!hasScope)
    {
        LOG_WARN("Attribute manager: attribute '" << id
                  << "' has no (valid) scope. Skipping...");
        delete attribute;
        return;
    }

    for_each_xml_child_node(subNode, attributeNode)
    {
        if (xmlStrEqual(subNode->name, BAD_CAST "modifier"))
        {
            readModifierNode(subNode, id, attribute);
        }
    }

    mAttributeMap[id] = attribute;
    mAttributeNameMap[name] = attribute;
}

/**
 * Check the status of recently loaded configuration.
 */
void AttributeManager::checkStatus()
{
    LOG_DEBUG("attribute map:");
    LOG_DEBUG("Stackable is " << Stackable << ", NonStackable is " << NonStackable
              << ", NonStackableBonus is " << NonStackableBonus << ".");
    LOG_DEBUG("Additive is " << Additive << ", Multiplicative is " << Multiplicative << ".");
    const std::string *tag;
    unsigned count = 0;
    for (auto &attributeIt : mAttributeMap)
    {
        unsigned lCount = 0;
        LOG_DEBUG("  "<< attributeIt.first<<" : ");
        for (auto &modifierIt : attributeIt.second->modifiers)
        {
            tag = getTag(ModifierLocation(attributeIt.first, lCount));
            std::string end = tag ? "tag of '" + (*tag) + "'." : "no tag.";
            LOG_DEBUG("    stackableType: " << modifierIt.stackableType
                      << ", effectType: " << modifierIt.effectType << ", and "
                      << end);
            ++lCount;
            ++count;
        }
    }
    LOG_INFO("Loaded '" << mAttributeMap.size() << "' attributes with '"
             << count << "' modifier layers.");

    for (auto &tagIt : mTagMap)
    {
        LOG_DEBUG("Tag '" << tagIt.first << "': '" << tagIt.second.attributeId
                  << "', '" << tagIt.second.layer << "'.");
    }

    LOG_INFO("Loaded '" << mTagMap.size() << "' modifier tags.");
}

void AttributeManager::readModifierNode(xmlNodePtr modifierNode,
                                        int attributeId, AttributeInfo *info)
{
    const std::string stackableTypeString = utils::toUpper(
                XML::getProperty(modifierNode, "stacktype", std::string()));
    const std::string effectTypeString = utils::toUpper(
                XML::getProperty(modifierNode, "modtype", std::string()));
    const std::string tag = XML::getProperty(modifierNode, "tag",
                                             std::string());

    if (stackableTypeString.empty())
    {
        LOG_WARN("Attribute manager: attribute '" << attributeId <<
                 "' has undefined stackable type, skipping modifier!");
        return;
    }

    if (effectTypeString.empty())
    {
        LOG_WARN("Attribute manager: attribute '" << attributeId
                 << "' has undefined modification type, skipping modifier!");
        return;
    }

    StackableType stackableType;
    ModifierEffectType effectType;

    if (stackableTypeString == "STACKABLE")
        stackableType = Stackable;
    else if (stackableTypeString == "NON STACKABLE")
        stackableType = NonStackable;
    else if (stackableTypeString == "NON STACKABLE BONUS")
        stackableType = NonStackableBonus;
    else
    {
        LOG_WARN("Attribute manager: attribute '"
                 << attributeId << "' has unknown stackable type '"
                 << stackableTypeString << "', skipping modifier!");
        return;
    }

    if (effectTypeString == "ADDITIVE")
        effectType = Additive;
    else if (effectTypeString == "MULTIPLICATIVE")
        effectType = Multiplicative;
    else
    {
        LOG_WARN("Attribute manager: attribute '" << attributeId
                 << "' has unknown modification type '"
                 << effectTypeString << "', skipping modifier!");
        return;
    }

    if (stackableType == NonStackable && effectType == Multiplicative)
    {
        LOG_WARN("Attribute manager: attribute '" << attributeId
                 << "' has a non sense modifier. "
                 << "Having NonStackable and Multiplicative makes no sense! "
                 << "Skipping modifier!");
        return;
    }

    info->modifiers.push_back(
                AttributeModifier(stackableType, effectType));

    if (!tag.empty())
    {
        const int layer = info->modifiers.size() - 1;
        mTagMap.insert(std::make_pair(tag, ModifierLocation(attributeId,
                                                            layer)));
    }
}
