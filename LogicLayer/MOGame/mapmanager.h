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

#ifndef MAPMANAGER_H
#define MAPMANAGER_H

#include <map>
#include <string>

#include "utils/xml.h"

class MapComposite;

namespace MapManager
{
    typedef std::map< int, MapComposite * > Maps;

    void initialize();

    void deinitialize();

    void reload();

    void readMapNode(xmlNodePtr node);

    void checkStatus();

    /**
     * Returns the requested map.
     *
     * @return the requested map, or nullptr if no map with the given ID exists.
     */
    MapComposite *getMap(int mapId);

    /**
     * Returns the requested map
     */
    MapComposite *getMap(const std::string &mapName);

    /**
     * Returns all the maps.
     */
    const Maps &getMaps();

    /**
     * Sets the activity status of the map.
     * @return true if the activation was successful.
     */
    bool activateMap(int mapId);
}

#endif // MAPMANAGER_H
