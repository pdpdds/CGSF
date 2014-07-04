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

#ifndef ACCOUNTHANDLER_H
#define ACCOUNTHANDLER_H

#include <string>

namespace AccountClientHandler
{
    /**
     * Creates a connection handler and starts listening on given port.
     */
    bool initialize(const std::string &configFile, int port,
                    const std::string &host);

    /**
     * Stops listening to messages and destroys the connection handler.
     */
    void deinitialize();

    /**
     * Prepares a connection for a client coming from a game server.
     */
    void prepareReconnect(const std::string &token, int accountID);

    /**
     * Processes messages received by the connection handler.
     */
    void process();
}

#endif // ACCOUNTHANDLER_H
