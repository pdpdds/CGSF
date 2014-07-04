/*
 *  The Mana Server
 *  Copyright (C) 2004-2010  The Mana World Development Team
 *  Copyright (C) 2010-2013  The Mana Development Team
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

#include "account-server/mapmanager.h"

#include "utils/logger.h"
#include "utils/xml.h"

#include <map>

static std::map<int, std::string> maps;

void MapManager::initialize(const std::string &mapReferenceFile)
{
    maps.clear();

    XML::Document doc(mapReferenceFile);
    xmlNodePtr rootNode = doc.rootNode();

    if (!rootNode || !xmlStrEqual(rootNode->name, BAD_CAST "maps"))
    {
        LOG_ERROR("Map Manager: Error while parsing map database ("
                  << mapReferenceFile << ")!");
        return;
    }
    LOG_INFO("Loading map reference: " << mapReferenceFile);
    for_each_xml_child_node(node, rootNode)
    {
        if (!xmlStrEqual(node->name, BAD_CAST "map"))
            continue;

        int id = XML::getProperty(node, "id", 0);
        std::string name = XML::getProperty(node, "servername", std::string());

        if (id > 0)
            maps[id] = name;
    }
}

std::map<int, std::string> &MapManager::getMaps()
{
    return maps;
}
