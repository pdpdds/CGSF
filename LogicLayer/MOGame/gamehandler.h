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

#ifndef SERVER_GAMEHANDLER_H
#define SERVER_GAMEHANDLER_H

#include "net/connectionhandler.h"
#include "net/netcomputer.h"
#include "utils/tokencollector.h"

class Entity;

enum
{
    CLIENT_LOGIN = 0,
    CLIENT_CONNECTED,
    CLIENT_CHANGE_SERVER,
    CLIENT_QUEUED
};

struct GameClient: NetComputer
{
    GameClient(ENetPeer *peer)
      : NetComputer(peer), character(nullptr), status(CLIENT_LOGIN) {}
    Entity *character;
    int status;
};

/**
 * Manages connections to game client.
 */
class GameHandler: public ConnectionHandler
{
    public:
        GameHandler();

        /**
         * Starts the handler
         */
        bool startListen(enet_uint16 port);

        /**
         * Sends message to the given character.
         */
        void sendTo(Entity *, MessageOut &msg);
        void sendTo(GameClient *, MessageOut &msg);

        /**
         * Kills connection with given character.
         */
        void detachClient(Entity *);

        /**
         * Prepares a server change for given character.
         */
        void prepareServerChange(Entity *);

        /**
         * Completes a server change for given character ID.
         */
        void completeServerChange(int id, const std::string &token,
                                  const std::string &address, int port);

        /**
         * Updates the party id of the character
         */
        void updateCharacter(int charid, int partyid);

        /**
         * Registers a character that should soon be claimed by a client.
         * @param token token used by the client when connecting.
         */
        void addPendingCharacter(const std::string &token, Entity *);

        /**
         * Combines a client with its character.
         * (Needed for TokenCollector)
         */
        void tokenMatched(GameClient *computer, Entity *character);

        /**
         * Deletes a pending client's data.
         * (Needed for TokenCollector)
         */
        void deletePendingClient(GameClient *computer);

        /**
         * Deletes a pending connection's data.
         * (Needed for TokenCollector)
         */
        void deletePendingConnect(Entity *character);

        /**
         * Gets the character associated to a character name. This method is
         * slow, so it should never be called for regular operations.
         */
        Entity *getCharacterByNameSlow(const std::string &) const;

    protected:
        NetComputer *computerConnected(ENetPeer *);
        void computerDisconnected(NetComputer *);

        /**
         * Processes messages related to core game events.
         */
        void processMessage(NetComputer *computer, MessageIn &message);

    private:
        void handleSay(GameClient &client, MessageIn &message);
        void handleNpc(GameClient &client, MessageIn &message);
        void handlePickup(GameClient &client, MessageIn &message);
        void handleUseItem(GameClient &client, MessageIn &message);
        void handleDrop(GameClient &client, MessageIn &message);
        void handleWalk(GameClient &client, MessageIn &message);

        void handleEquip(GameClient &client, MessageIn &message);
        void handleUnequip(GameClient &client, MessageIn &message);

        void handleUseAbilityOnBeing(GameClient &client, MessageIn &message);
        void handleUseAbilityOnPoint(GameClient &client, MessageIn &message);
        void handleUseAbilityOnDirection(GameClient &client, MessageIn &message);

        void handleActionChange(GameClient &client, MessageIn &message);
        void handleDirectionChange(GameClient &client, MessageIn &message);

        void handleDisconnect(GameClient &client, MessageIn &message);

        void handleTradeRequest(GameClient &client, MessageIn &message);
        void handleTrade(GameClient &client, MessageIn &message);

        void handleNpcBuySell(GameClient &client, MessageIn &message);

        void handleRaiseAttribute(GameClient &client, MessageIn &message);
        void handleLowerAttribute(GameClient &client, MessageIn &message);

        void handleNpcPostSend(GameClient &client, MessageIn &message);

        void handlePartyInvite(GameClient &client, MessageIn &message);

        void handleTriggerEmoticon(GameClient &client, MessageIn &message);

        void sendNpcError(GameClient &client, int id,
                          const std::string &errorMsg);

        /**
         * Container for pending clients and pending connections.
         */
        TokenCollector<GameHandler, GameClient *, Entity *> mTokenCollector;
};

extern GameHandler *gameHandler;

#endif
