/*
 *  The Mana Server
 *  Copyright (C) 2004-2010  The Mana World Development Team
 *  Copyright (C) 2010-2011  The Mana Development Team
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

#include "mapmanager.h"

#include "common/resourcemanager.h"
#include "common/defines.h"
#include "map.h"
#include "mapcomposite.h"
#include "utils/logger.h"

#include <cassert>

/**
 * List of all the game maps, be they present or not on this server.
 */
static MapManager::Maps maps;

const MapManager::Maps &MapManager::getMaps()
{
    return maps;
}

void MapManager::initialize()
{

}

/**
 * Destroys all maps.
 */
void MapManager::deinitialize()
{
    for (Maps::iterator i = maps.begin(), i_end = maps.end(); i != i_end; ++i)
    {
        delete i->second;
    }
    maps.clear();
}

/**
 * Prepare map manager for a reload.
 */
void MapManager::reload()
{
    // TODO: this method needs proper map reloading
    LOG_ERROR("MapManager::reload() not implemented yet");
}

/**
 * Read a <map> node from settings
 */
void MapManager::readMapNode(xmlNodePtr node)
{
    int id = XML::getProperty(node, "id", 0);
    std::string name = XML::getProperty(node, "name", std::string());


    if (id <= 0)
    {
        LOG_WARN("Invalid map Id: " << id << " for map: "
                 << name << '.');
    }
    else if (name.empty())
    {
        LOG_WARN("Invalid unnamed map Id: " << id << '.');
    }
    else
    {
        // Testing if the file is actually in the maps folder
        std::string file = std::string("maps/") + name + ".tmx";
        bool mapFileExists = ResourceManager::exists(file);

        // Try to fall back on fully compressed map
        if (!mapFileExists)
        {
            file += ".gz";
            mapFileExists = ResourceManager::exists(file);
        }

        if (mapFileExists)
        {
            maps[id] = new MapComposite(id, name);
            if (!maps[id]->readMap())
                LOG_FATAL("Failed to load map \"" << name << "\"!");
        }
    }
}

/**
 * Check the status of recently loaded configuration.
 */
void MapManager::checkStatus()
{
    int loadedMaps = maps.size();
    if (loadedMaps > 0)
    {
        LOG_INFO(loadedMaps << " valid map file references were loaded.");
    }
    else
    {
        LOG_FATAL("The Game Server can't find any valid/available maps.");
        exit(EXIT_MAP_FILE_NOT_FOUND);
    }
}

MapComposite *MapManager::getMap(int mapId)
{
    Maps::const_iterator i = maps.find(mapId);
    return (i != maps.end()) ? i->second : nullptr;
}

MapComposite *MapManager::getMap(const std::string &mapName)
{
    for (Maps::const_iterator i = maps.begin(); i != maps.end(); ++i)
        if (i->second->getName() == mapName)
            return i->second;

    return nullptr;
}

bool MapManager::activateMap(int mapId)
{
    Maps::iterator i = maps.find(mapId);
    assert(i != maps.end());
    MapComposite *composite = i->second;

    if (composite->isActive())
        return true;

    if (composite->activate())
    {
        LOG_INFO("Activated map \"" << composite->getName()
                 << "\" (id " << mapId << ")");
        return true;
    }
    else
    {
        LOG_WARN("Couldn't activate invalid map \"" << composite->getName()
                 << "\" (id " << mapId << ")");
        return false;
    }
}
