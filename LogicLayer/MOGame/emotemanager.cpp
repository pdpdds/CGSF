/*
 *  The Mana Server
 *  Copyright (C) 2012  The Mana Developers
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

#include "emotemanager.h"

#include "utils/xml.h"
#include "utils/logger.h"

void EmoteManager::initialize()
{
    clear();
}

void EmoteManager::reload()
{
    clear();
}

/**
 * Read a <emote> element from settings.
 * Used by SettingsManager.
 */
void EmoteManager::readEmoteNode(xmlNodePtr node, const std::string &filename)
{
    int id = XML::getProperty(node, "id", -1);
    if (id < 0)
    {
        LOG_WARN("The " << filename << " file is containing an invalid id"
                 "(" << id << ") and will be ignored.");
        return;
    }

    mEmoteIds.push_back(id);
}

/**
 * Check the status of recently loaded configuration.
 */
void EmoteManager::checkStatus()
{
    LOG_INFO(mEmoteIds.size() << " emotes available.");
}


bool EmoteManager::isIdAvailable(int id) const
{
    std::vector<int>::const_iterator it = mEmoteIds.begin();
    std::vector<int>::const_iterator it_end = mEmoteIds.end();
    for (; it != it_end; ++it)
    {
        if ((*it) == id)
            return true;
    }
    return false;
}
