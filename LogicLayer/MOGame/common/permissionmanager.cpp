/*
 *  The Mana Server
 *  Copyright (C) 2010-2010  The Mana World Development Team
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

#include "common/permissionmanager.h"

#include "charactercomponent.h"
#include "utils/logger.h"
#include "utils/xml.h"

#include <string.h>
#include <cstring>

static std::map<std::string, unsigned char> permissions;
static std::map<std::string, unsigned char> aliases;
static std::string permissionFile;

void addPermission(std::string permission, char mask)
{

    std::map<std::string, unsigned char>::iterator i = permissions.find(permission);
    if (i == permissions.end())
    {
        permissions.insert(std::make_pair(permission, mask));
    } else {
        i->second |= mask;
    }
}

void PermissionManager::initialize(const std::string & file)
{
    permissionFile = file;
    reload();
}

void PermissionManager::reload()
{
    XML::Document doc(permissionFile);
    xmlNodePtr rootNode = doc.rootNode();

    if (!rootNode || !xmlStrEqual(rootNode->name, BAD_CAST "permissions"))
    {
        LOG_ERROR("Permission Manager: " << permissionFile
                  << " is not a valid database file!");
        return;
    }

    LOG_INFO("Loading permission reference...");
    for_each_xml_child_node(node, rootNode)
    {
        unsigned char classmask = 0x01;
        if (!xmlStrEqual(node->name, BAD_CAST "class"))
        {
            continue;
        }
        int level = XML::getProperty(node, "level", 0);
        if (level < 1 || level > 8)
        {
            LOG_WARN("PermissionManager: Illegal class level "
                    <<level
                    <<" in "
                    <<permissionFile
                    <<" (allowed range: 1..8)");
            continue;
        }
        classmask = classmask << (level-1);


        xmlNodePtr perNode;
        for (perNode = node->xmlChildrenNode; perNode != nullptr; perNode = perNode->next)
        {
            if (xmlStrEqual(perNode->name, BAD_CAST "allow"))
            {
                const char* permission = (const char*)perNode->xmlChildrenNode->content;
                if (permission && strlen(permission) > 0)
                {
                    addPermission(permission, classmask);
                }
            } else if (xmlStrEqual(perNode->name, BAD_CAST "deny")){
                //const char* permission = (const char*)perNode->xmlChildrenNode->content;
                // To be implemented
            } else if (xmlStrEqual(perNode->name, BAD_CAST "alias")){
                const char* alias = (const char*)perNode->xmlChildrenNode->content;
                if (alias && strlen(alias) > 0)
                aliases[alias] = classmask;
            }
        }
    }
}


PermissionManager::Result PermissionManager::checkPermission(const Entity* character, std::string permission)
{
    return checkPermission(character->getComponent<CharacterComponent>()
                               ->getAccountLevel(), permission);
}

PermissionManager::Result PermissionManager::checkPermission(unsigned char level, std::string permission)
{
    std::map<std::string, unsigned char>::iterator iP = permissions.find(permission);

    if (iP == permissions.end())
    {
        LOG_WARN("PermissionManager: Check for unknown permission \""<<permission<<"\" requested.");
        return PMR_UNKNOWN;
    }
    if (level & iP->second)
    {
        return PMR_ALLOWED;
    } else {
        return PMR_DENIED;
    }
}

unsigned char PermissionManager::getMaskFromAlias(const std::string &alias)
{
    std::map<std::string, unsigned char>::iterator i = aliases.find(alias);

    if (i == aliases.end())
    {
        return 0x00;
    } else {
        return i->second;
    }
}

std::list<std::string> PermissionManager::getPermissionList(const Entity* character)
{
    std::list<std::string> result;

    unsigned char mask = character->getComponent<CharacterComponent>()
            ->getAccountLevel();

    for (auto &permissionIt : permissions)
    {
        if (permissionIt.second & mask)
        {
            result.push_back(permissionIt.first);
        }
    }

    return result;
}

std::list<std::string> PermissionManager::getClassList(const Entity* character)
{
    std::list<std::string> result;

    unsigned char mask = character->getComponent<CharacterComponent>()
            ->getAccountLevel();

    for (auto &aliasIt : aliases)
    {
        if (aliasIt.second & mask)
            result.push_back(aliasIt.first);
    }

    return result;
}

