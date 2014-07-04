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

#ifndef STATUSMANAGER_H
#define STATUSMANAGER_H

#include <string>
#include "utils/xml.h"

class StatusEffect;

namespace StatusManager
{
    /**
     * Loads status reference file.
     */
    void initialize();

    /**
     * Reloads status reference file.
     */
    void reload();

    /**
     * Destroy status classes.
     */
    void deinitialize();

    /**
     * Gets the status having the given ID.
     */
    StatusEffect *getStatus(int statusId);

    /**
     * Gets the status having the given name.
     */
    StatusEffect *getStatusByName(const std::string &name);

    void readStatusNode(xmlNodePtr node, const std::string &filename);

    void checkStatus();
}

#endif // STATUSMANAGER_H
