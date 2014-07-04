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

#ifndef CHATCLIENT_H
#define CHATCLIENT_H

#include <map>
#include <string>
#include <vector>

#include "net/netcomputer.h"

class ChatChannel;
class Guild;
class Party;

/**
 * A client connected to the chat server. Via this class, the chat server
 * keeps track of the character name and account level of a client.
 */
class ChatClient : public NetComputer
{
    public:
        ChatClient(ENetPeer *peer)
            : NetComputer(peer)
            , characterId(0)
            , party(0)
            , accountLevel(0)
        {
        }

        std::string characterName;
        unsigned characterId;
        std::vector<ChatChannel *> channels;
        std::vector<Guild *> guilds;
        Party *party;
        unsigned char accountLevel;
        std::map<ChatChannel*, std::string> userModes;
};

#endif
