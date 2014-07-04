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

#ifndef CHATCHANNELMANAGER_H
#define CHATCHANNELMANAGER_H

#include <list>
#include <map>
#include <deque>

#include "chat-server/chatchannel.h"

/**
 * The chat channel manager takes care of registering and removing public and
 * private chat channels. Every channel gets a unique channel ID.
 */
class ChatChannelManager
{
    public:
        ChatChannelManager();
        ~ChatChannelManager();

        /**
         * Create a new chat channel.
         *
         * @return the ID of the registered channel
         */
        int createNewChannel(const std::string &channelName,
                             const std::string &channelAnnouncement,
                             const std::string &channelPassword,
                             bool joinable);

        /**
         * Try to create a new public channel with the given name.
         *
         * @return true if created successfully, false otherwise.
         */
        bool tryNewPublicChannel(const std::string &name);

        /**
         * Remove a channel.
         */
        bool removeChannel(int channelId);

        /**
         * Returns a list containing all public channels.
         *
         * @return a list of all public channels
         */
        std::list<const ChatChannel*> getPublicChannels() const;

        /**
         * Get the id of a channel from its name.
         *
         * @return the id of the channel, 0 if it was unsuccessful.
         */
        int getChannelId(const std::string &channelName) const;

        /**
         * Returns the chat channel with the given channel ID.
         *
         * @return The chat channel, or nullptr when it doesn't exist.
         */
        ChatChannel *getChannel(int channelId);

        /**
         * Returns the chat channel with the given channel name.
         *
         * @return The chat channel, or nullptr when it doesn't exist.
         */
        ChatChannel *getChannel(const std::string &name);

        /**
         * Remove a user from all channels. Used at logout.
         *
         * @see ChatChannel::removeUserFromChannel
         */
        void removeUserFromAllChannels(ChatClient *);

        /**
         * Set the topic of a channel.
         *
         * @param topic the new topic of the given channel
         */
        void setChannelTopic(int channelId, const std::string &name);

        /**
         * Returns whether a channel exists.
         *
         * @param channelId a channel ID
         */
        bool channelExists(int channelId) const;
        bool channelExists(const std::string &channelName) const;

        /**
         * Get next usable channel ID
         */
        int nextUsable();

    private:
        typedef std::map<unsigned short, ChatChannel> ChatChannels;

        /**
         * The map keeping all the chat channels. The channel id must be
         * unique.
         */
        ChatChannels mChatChannels;
        int mNextChannelId;
        std::deque<int> mChannelsNoLongerUsed;
};

extern ChatChannelManager *chatChannelManager;

#endif
