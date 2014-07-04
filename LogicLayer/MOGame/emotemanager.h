/*
 *  The Mana Server
 *  Copyright (C) 2011-2012  The Mana Developers
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


#ifndef EMOTEMANAGER_H
#define EMOTEMANAGER_H

#include "utils/string.h"
#include "utils/xml.h"

#include <vector>

class EmoteManager
{
public:

    EmoteManager()
    { }

    ~EmoteManager()
    { clear(); }

    /**
     * Loads emote reference file.
     */
    void initialize();

    void reload();

    /**
     * Tells whether the given id is a valid emote one.
     */
    bool isIdAvailable(int id) const;

    void readEmoteNode(xmlNodePtr node, const std::string &filename);

    void checkStatus();

private:
    /**
     * Clears up the emote list.
     */
    void clear()
    { mEmoteIds.clear(); }

    std::vector<int> mEmoteIds;
};

extern EmoteManager *emoteManager;

#endif // EMOTEMANAGER_H
