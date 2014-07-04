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

#include <algorithm>
#include <sstream>

#include "chat-server/chatchannel.h"
#include "chat-server/chatclient.h"

ChatChannel::ChatChannel(int id,
                         const std::string &name,
                         const std::string &announcement,
                         const std::string &password,
                         bool joinable):
    mId(id),
    mName(name),
    mAnnouncement(announcement),
    mPassword(password),
    mJoinable(joinable)
{
}

bool ChatChannel::addUser(ChatClient *user)
{
    // First user is the channel owner
    if (mRegisteredUsers.size() < 1)
    {
        mOwner = user->characterName;
        setUserMode(user, 'o');
    }

    // Check if the user already exists in the channel
    ChannelUsers::const_iterator i = mRegisteredUsers.begin(),
                                 i_end = mRegisteredUsers.end();
    if (std::find(i, i_end, user) != i_end)
        return false;

    mRegisteredUsers.push_back(user);
    user->channels.push_back(this);

    // set user as logged in
    setUserMode(user, 'l');

    // if owner has rejoined, give them ops
    if (user->characterName == mOwner)
    {
        setUserMode(user, 'o');
    }

    return true;
}

bool ChatChannel::removeUser(ChatClient *user)
{
    ChannelUsers::iterator i_end = mRegisteredUsers.end(),
                           i = std::find(mRegisteredUsers.begin(), i_end, user);
    if (i == i_end) return false;
    mRegisteredUsers.erase(i);
    std::vector< ChatChannel * > &channels = user->channels;
    channels.erase(std::find(channels.begin(), channels.end(), this));
    std::map<ChatChannel*,std::string> &modes = user->userModes;
    modes.erase(modes.begin(), modes.end());
    return true;
}

void ChatChannel::removeAllUsers()
{
    for (ChannelUsers::const_iterator i = mRegisteredUsers.begin(),
         i_end = mRegisteredUsers.end(); i != i_end; ++i)
    {
        std::vector< ChatChannel * > &channels = (*i)->channels;
        channels.erase(std::find(channels.begin(), channels.end(), this));
        std::map<ChatChannel*,std::string> &modes = (*i)->userModes;
        modes.erase(modes.begin(), modes.end());
    }
    mRegisteredUsers.clear();
}

bool ChatChannel::canJoin() const
{
    return mJoinable;
}

void ChatChannel::setUserMode(ChatClient *user, unsigned char mode)
{
    std::map<ChatChannel*, std::string>::iterator itr = user->userModes.find(this);
    if (itr != user->userModes.end())
    {
        itr->second += mode;
    }
    else
    {
        std::stringstream ss; ss << mode;
        user->userModes.insert(std::pair<ChatChannel*, std::string>(this, ss.str()));
    }
}

std::string ChatChannel::getUserMode(ChatClient *user) const
{
    std::map<ChatChannel*, std::string>::const_iterator itr =
            user->userModes.find(const_cast<ChatChannel*>(this));

    if (itr != user->userModes.end())
        return itr->second;

    return 0;
}
