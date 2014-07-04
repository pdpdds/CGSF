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

#include <list>

#include "chat-server/chatchannelmanager.h"

#include "account-server/storage.h"
#include "chat-server/chatclient.h"
#include "chat-server/chathandler.h"
#include "chat-server/guildmanager.h"
#include "common/configuration.h"
#include "common/manaserv_protocol.h"
#include "utils/stringfilter.h"

using namespace ManaServ;

ChatChannelManager::ChatChannelManager() : mNextChannelId(1)
{
}


ChatChannelManager::~ChatChannelManager()
{
}

int ChatChannelManager::createNewChannel(const std::string &channelName,
                                         const std::string &channelAnnouncement,
                                         const std::string &channelPassword,
                                         bool joinable)
{
    int channelId = nextUsable();

    // Register channel
    mChatChannels.insert(std::make_pair(channelId,
                                        ChatChannel(channelId,
                                                    channelName,
                                                    channelAnnouncement,
                                                    channelPassword,
                                                    joinable)));
    return channelId;
}

bool ChatChannelManager::tryNewPublicChannel(const std::string &name)
{
    if (!stringFilter->filterContent(name))
    {
        return false;
    }

    // Checking strings for length and double quotes
    unsigned maxNameLength = Configuration::getValue("chat_maxChannelNameLength", 15);
    if (name.empty() ||
        name.length() > maxNameLength ||
        stringFilter->findDoubleQuotes(name))
    {
        return false;
    }
    else if (guildManager->doesExist(name) ||
             channelExists(name))
    {
        // Channel already exists
        return false;
    }
    else
    {
        // We attempt to create a new channel
        short id = createNewChannel(name, std::string(), std::string(), true);
        return id != 0;
    }
}

bool ChatChannelManager::removeChannel(int channelId)
{
    ChatChannels::iterator i = mChatChannels.find(channelId);
    if (i == mChatChannels.end())
        return false;
    i->second.removeAllUsers();
    mChatChannels.erase(i);
    mChannelsNoLongerUsed.push_back(channelId);
    return true;
}

std::list<const ChatChannel*> ChatChannelManager::getPublicChannels() const
{
    std::list<const ChatChannel*> channels;

    for (ChatChannels::const_iterator i = mChatChannels.begin(),
            i_end = mChatChannels.end();
         i != i_end; ++i)
    {
        if (i->second.canJoin())
        {
            channels.push_back(&i->second);
        }
    }

    return channels;
}

int ChatChannelManager::getChannelId(const std::string &channelName) const
{
    for (ChatChannels::const_iterator i = mChatChannels.begin(),
            i_end = mChatChannels.end();
         i != i_end; ++i)
    {
        if (i->second.getName() == channelName)
            return i->first;
    }
    return 0;
}

ChatChannel *ChatChannelManager::getChannel(int channelId)
{
    ChatChannels::iterator i = mChatChannels.find(channelId);
    if (i != mChatChannels.end())
        return &i->second;
    return 0;
}

ChatChannel *ChatChannelManager::getChannel(const std::string &name)
{
    for (ChatChannels::iterator i = mChatChannels.begin();
         i != mChatChannels.end(); ++i)
    {
        if (i->second.getName() == name)
            return &(i->second);
    }

    return 0;
}

void ChatChannelManager::setChannelTopic(int channelId, const std::string &topic)
{
    ChatChannels::iterator i = mChatChannels.find(channelId);
    if (i == mChatChannels.end())
        return;

    i->second.setAnnouncement(topic);
    chatHandler->warnUsersAboutPlayerEventInChat(&(i->second),
                                                 topic,
                                                 CHAT_EVENT_TOPIC_CHANGE);
}

void ChatChannelManager::removeUserFromAllChannels(ChatClient *user)
{
    // Local copy as they will be destroyed under our feet.
    std::vector<ChatChannel *> channels = user->channels;
    // Reverse iterator to reduce load on vector operations.
    for (std::vector<ChatChannel *>::const_reverse_iterator
         i = channels.rbegin(), i_end = channels.rend(); i != i_end; ++i)
    {
        chatHandler->warnUsersAboutPlayerEventInChat((*i),
                                                     user->characterName,
                                                     CHAT_EVENT_LEAVING_PLAYER);
        (*i)->removeUser(user);
    }
}

bool ChatChannelManager::channelExists(int channelId) const
{
    return mChatChannels.find(channelId) != mChatChannels.end();
}

bool ChatChannelManager::channelExists(const std::string &channelName) const
{
    for (ChatChannels::const_iterator i = mChatChannels.begin();
         i != mChatChannels.end(); ++i)
    {
        if (i->second.getName() == channelName)
            return true;
    }
    return false;
}

int ChatChannelManager::nextUsable()
{
    int channelId = 0;

    if (mChannelsNoLongerUsed.size() > 0)
    {
        channelId = mChannelsNoLongerUsed[0];
        mChannelsNoLongerUsed.pop_front();
    }
    else
    {
        channelId = mNextChannelId;
        ++mNextChannelId;
    }

    return channelId;
}
