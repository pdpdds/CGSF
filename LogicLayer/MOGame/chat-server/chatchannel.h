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

#ifndef CHATCHANNEL_H
#define CHATCHANNEL_H

#include <string>
#include <vector>

class ChatClient;

/**
 * A chat channel. Optionally a channel is private, in which case a password is
 * required to join it.
 *
 * No logic is currently associated with a chat channel except for making sure
 * that no user joins the channel twice and checking that a user who leaves
 * actually existed in the channel.
 *
 * @todo <b>b_lindeijer:</b> It would be nicer when some more logic could be
 *       placed in this class to remove some weight from the ChatHandler.
 */
class ChatChannel
{
    public:
        typedef std::vector< ChatClient * > ChannelUsers;

        /**
         * Constructor.
         *
         * @param name         the name of the channel.
         * @param announcement a welcome message.
         * @param password     password (for private channels).
         * @param privacy      whether this channel is private.
         */
        ChatChannel(int id,
                    const std::string &name,
                    const std::string &announcement = std::string(),
                    const std::string &password = std::string(),
                    bool joinable = true);

        /**
         * Get the ID of the channel.
         */
        int getId() const
        { return mId; }

        /**
         * Get the name of the channel.
         */
        const std::string &getName() const
        { return mName; }

        /**
         * Get the announcement string of the channel.
         */
        const std::string &getAnnouncement() const
        { return mAnnouncement; }

        /**
         * Get the password of the channel.
         */
        const std::string& getPassword() const
        { return mPassword; }

        /**
         * Sets the name of the channel.
         */
        void setName(const std::string &channelName)
        { mName = channelName; }

        /**
         * Sets the announcement string of the channel.
         */
        void setAnnouncement(const std::string &channelAnnouncement)
        { mAnnouncement = channelAnnouncement; }

        /**
         * Sets the password of the channel.
         */
        void setPassword(const std::string &channelPassword)
        { mPassword = channelPassword; }

        /**
         * Gets the list of the users registered in the channel.
         */
        const ChannelUsers &getUserList() const
        { return mRegisteredUsers; }

        /**
         * Adds a user to the channel.
         *
         * @return whether the user was successfully added
         */
        bool addUser(ChatClient *);

        /**
         * Removes a user from the channel.
         *
         * @return whether the user was successfully removed
         */
        bool removeUser(ChatClient *);

        /**
         * Empties a channel from its users (admin included).
         */
        void removeAllUsers();

        /**
         * Get whether a user can join this channel
         */
        bool canJoin() const;

        /**
         * Set user mode
         */
        void setUserMode(ChatClient *, unsigned char mode);

        /**
         * Get user mode
         */
        std::string getUserMode(ChatClient *) const;

    private:
        unsigned short mId;            /**< The ID of the channel. */
        std::string mName;             /**< The name of the channel. */
        std::string mAnnouncement;     /**< Welcome message. */
        std::string mPassword;         /**< The channel password. */
        bool mJoinable;                /**< Whether anyone can join. */
        ChannelUsers mRegisteredUsers; /**< Users in this channel. */
        std::string mOwner;             /**< Channel owner character name */
};

#endif
