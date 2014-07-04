/*
 *  The Mana Server
 *  Copyright (C) 2006-2010  The Mana World Development Team
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

#ifndef SERVERHANDLER_H
#define SERVERHANDLER_H

#include <iosfwd>
#include <string>

#include "net/messagein.h"

class CharacterData;

namespace GameServerHandler
{
    /**
     * Creates a connection handler and starts listening on given port.
     */
    bool initialize(int port, const std::string &host);

    /**
     * Stops listening to messages and destroys the connection handler.
     */
    void deinitialize();

    /**
     * Returns the information a client needs to connect to the game server
     * corresponding to the given map ID.
     */
    bool getGameServerFromMap(int, std::string &address, int &port);

    /**
     * Warns a game server about a soon-to-connect client.
     */
    void registerClient(const std::string &token, CharacterData *);

    /**
     * Dumps per-server statistics into given stream
     */
    void dumpStatistics(std::ostream &);

    /**
     * Processes messages received by the connection handler.
     */
    void process();

    /**
     * Sends chat party information
     */
    void sendPartyChange(CharacterData *ptr, int partyId);

    /**
     * Takes a GAMSG_PLAYER_SYNC from the gameserver and stores all changes in
     * the database.
     */
    void syncDatabase(MessageIn &msg);
}

#endif // SERVERHANDLER_H
