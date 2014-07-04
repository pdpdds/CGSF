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

#ifndef _MAPREADER_H
#define _MAPREADER_H

#include <string>
#include <vector>

#include <libxml/tree.h>

class Map;

/**
 * Reader for XML map files (*.tmx)
 */
class MapReader
{
    public:
        /**
         * Read an XML map from a file.
         * @return the map when successful, 0 otherwise.
         */
        static Map *readMap(const std::string &filename);

    private:
        /**
         * Read an XML map from a parsed XML tree.
         */
        static Map *readMap(xmlNodePtr node);

        /**
         * Reads a map layer and adds it to the given map.
         */
        static void readLayer(xmlNodePtr node, Map *map);

        /**
         * Get the string value from the given object property node.
         */
        static std::string getObjectProperty(xmlNodePtr node,
                                             const std::string &def);

        /**
         * Get the integer value from the given object property node.
         */
        static int getObjectProperty(xmlNodePtr node, int def);

        static void setTileWithGid(Map *map, int x, int y, unsigned gid);
};

#endif
