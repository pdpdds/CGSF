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

#ifndef MAPMANAGER_H
#define MAPMANAGER_H

#include <string>
#include <map>

namespace MapManager
{
    /**
     * Loads map reference file
     */
    void initialize(const std::string &mapReferenceFile);

    /**
     * Returns a map of all maps
     * @return a map of mapid + name of reserved server.
     */
    std::map<int, std::string> &getMaps();
}

#endif // MAPMANAGER_H
