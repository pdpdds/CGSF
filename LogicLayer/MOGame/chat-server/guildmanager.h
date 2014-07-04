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

#ifndef CHATSERVER_GUILDMANAGER_H
#define CHATSERVER_GUILDMANAGER_H

#include <list>
#include <string>
#include <vector>
#include <map>

class Guild;
class ChatClient;

/**
 * Guild manager takes care of creating, removing and modifying guilds.
 */
class GuildManager
{
    public:
        GuildManager();
        ~GuildManager();

        /**
         * Creates a guild.
         */
        Guild *createGuild(const std::string &name, int playerId);

        /**
         * Removes a guild.
         */
        void removeGuild(Guild *guild);

        /**
         * Adds a member to a guild.
         */
        void addGuildMember(Guild *guild, int playerId);

        /**
         * Removes a member from a guild.
         */
        void removeGuildMember(Guild *guild, int playerId,
                               const std::string &characterName,
                               ChatClient *client = 0);

        /**
         * Returns the guild with the given id. O(n)
         *
         * @return the guild with the given id, or nullptr if it doesn't exist
         */
        Guild *findById(short id) const;

        /**
         * Returns the guild with the given name. O(n)
         *
         * @return the guild with the given name, or nullptr if it doesn't exist
         */
        Guild *findByName(const std::string &name) const;

        /**
         * Returns whether a guild exists.
         */
        bool doesExist(const std::string &name) const;

        /**
         * Return the guilds a character is in
         */
        std::vector<Guild *> getGuildsForPlayer(int playerId) const;

        /**
         * Inform guild members that a player has disconnected.
         */
        void disconnectPlayer(ChatClient* player);

        /**
         * Promote a guild member to higher level or
         * Demote a guild member to a lower level
         *
         * @return Returns 0 if successful, -1 otherwise
         */
        int changeMemberLevel(ChatClient *player, Guild *guild,
                              int playerId, int level);

        /**
         * Set user rights
         */
        void setUserRights(Guild *guild, int playerId, int rights);

    private:
        std::map<int, Guild*> mGuilds;
};

extern GuildManager *guildManager;

#endif
