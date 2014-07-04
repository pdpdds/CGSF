/*
 *  The Mana Server
 *  Copyright (C) 2004-2010  The Mana World Development Team
 *  Copyright (C) 2010  The Mana Developers
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

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <string>

namespace Configuration
{
    /**
     * Loads the configuration options into memory.
     *
     * @param filename path to the configuration file. When empty, the default
     *                 config file 'manaserv.xml' is used.
     * @return whether the configuration file could be read
     */
    bool initialize(const std::string &fileName = std::string());

    void deinitialize();

    /**
     * Gets an option as a string.
     * @param key option identifier.
     * @param deflt default value.
     */
    std::string getValue(const std::string &key, const std::string &deflt);

    /**
     * Gets an option as an integer.
     * @param key option identifier.
     * @param deflt default value.
     */
    int getValue(const std::string &key, int deflt);

    /**
     * Gets an option as a boolean.
     * @param key option identifier.
     * @param deflt default value.
     */
    bool getBoolValue(const std::string &key, bool deflt);
}

#ifndef DEFAULT_SERVER_PORT
#define DEFAULT_SERVER_PORT 9601
#endif

#endif // CONFIGURATION_H
