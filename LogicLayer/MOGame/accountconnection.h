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

#ifndef ACCOUNTCONNECTION_H
#define ACCOUNTCONNECTION_H

#include "net/messageout.h"
#include "net/connection.h"

class Entity;
class MapComposite;

/**
 * A connection to the account server.
 */
class AccountConnection : public Connection
{
    public:
        AccountConnection();
        ~AccountConnection();

        /**
         * Initializes a connection to the account server described in the
         * configuration file. Registers the maps known by MapManager.
         */
        bool start(int gameServerPort);

        /**
         * Sends data of a given character.
         */
        void sendCharacterData(Entity *);

        /**
         * Prepares the account server for a reconnecting player
         */
        void playerReconnectAccount(int id, const std::string &magic_token);

        /**
         * Requests the value of a character-bound variable from the database.
         */
        void requestCharacterVar(Entity *, const std::string &);

        /**
         * Pushes a new character-bound value to the database.
         */
        void updateCharacterVar(Entity *, const std::string &name,
                                const std::string &value);

        /**
         * Pushes a new value of a map variable to the account server.
         */
        void updateMapVar(MapComposite *, const std::string &name,
                          const std::string &value);

        /**
         * Pushes a new value of a world variable to the account server.
         */
        void updateWorldVar(const std::string &name,
                            const std::string &value);

        /**
         * Sends ban message.
         */
        void banCharacter(Entity *, int);

        /**
         * Gathers statistics and sends them.
         */
        void sendStatistics();

        /**
         * Send letter
         */
        void sendPost(Entity *, MessageIn &);

        /**
         * Get post
         */
        void getPost(Entity *);

        /**
         * Change Account Level
         */
        void changeAccountLevel(Entity *, int);

        /**
         * Sends all changed player data to the account server to minimize
         * dataloss due to failure of one server component.
         *
         * The gameserver holds a buffer with all changes made by a character.
         * The changes are added at the time they occur. When the buffer
         * reaches one of the following limits, the buffer is sent to the
         * account server and applied to the database.
         *
         * The sync buffer is sent when:
         * - forced by any process (param force = true)
         * - every 10 seconds
         * - buffer reaches size of 1kb (SYNC_BUFFER_SIZE)
         * - buffer holds more then 20 messages (SYNC_BUFFER_LIMIT)
         *
         * @param force Send changes even if buffer hasn't reached its size
         *              or message limit. (used to send in timed schedules)
         */
        void syncChanges(bool force = false);

        /**
         * Write a modification message about character points to the sync
         * buffer.
         *
         * @param charId      ID of the character
         * @param charPoints  character points left for the character
         * @param corrPoints  correction points left for the character
         */
        void updateCharacterPoints(int charId, int charPoints,
                                   int corrPoints);

        /**
         * @param charId      ID of the character
         * @param attribId    ID of the modified attribute
         * @param attribValue New value of the modified attribute
         */

        void updateAttributes(int charId, int attrId, double base,
                              double mod);

        /**
         * Update the status of a character to online (true) or offline
         * (false).
         *
         * @param charId Id of the character.
         * @param online True to flag the character as being online.
         */
        void updateOnlineStatus(int charId, bool online);

        /**
         * Adds floor items info on database.
         *
         * This is used to make them potentially persistent between two server
         * restart.
         */
        void createFloorItems(int mapId, int itemId, int amount,
                              int posX, int posY);

        /**
         * Remove floor items from the database
         */
        void removeFloorItems(int mapId, int itemId, int amount,
                              int posX, int posY);

        /**
         * Send transaction to account server
         */
        void sendTransaction(int id, int action, const std::string &message);

    protected:
        /**
         * Processes server messages.
         */
        virtual void processMessage(MessageIn &);

    private:
        MessageOut* mSyncBuffer;     /**< Message buffer to store sync data. */
        int mSyncMessages;           /**< Number of messages in the sync buffer. */
};

extern AccountConnection *accountHandler;

#endif // ACCOUNTCONNECTION_H
