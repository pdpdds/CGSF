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

#include "guild.h"

#include "chat-server/guildmanager.h"

#include "common/defines.h"

#include <algorithm>

Guild::Guild(const std::string &name)
    : mId(0)
    , mName(name)
{
}

Guild::~Guild()
{
}

void Guild::addMember(int playerId, int permissions)
{
    // create new guild member
    GuildMember *member = new GuildMember;
    member->mId = playerId;
    member->mPermissions = permissions;

    // add new guild member to guild
    mMembers.push_back(member);

    mInvited.remove(playerId);
}

void Guild::removeMember(int playerId)
{
    if (getOwner() == playerId)
    {
        // if the leader is leaving, assign next member as leader
        for (std::list<GuildMember*>::iterator it = mMembers.begin(),
             it_end = mMembers.end(); it != it_end; ++it)
        {
            GuildMember *member = *it;
            if (member->mId != playerId)
            {
                setOwner(member->mId);
                break;
            }
        }
    }
    GuildMember *member = getMember(playerId);
    if (member)
        mMembers.remove(member);
}

int Guild::getOwner() const
{
    std::list<GuildMember*>::const_iterator itr = mMembers.begin();
    std::list<GuildMember*>::const_iterator itr_end = mMembers.end();

    while (itr != itr_end)
    {
        if ((*itr)->mPermissions == GAL_OWNER)
            return (*itr)->mId;
        ++itr;
    }

    return 0;
}

void Guild::setOwner(int playerId)
{
    guildManager->setUserRights(this, playerId, GAL_OWNER);
}

bool Guild::checkInvited(int playerId) const
{
    return std::find(mInvited.begin(), mInvited.end(), playerId) != mInvited.end();
}

void Guild::addInvited(int playerId)
{
    mInvited.push_back(playerId);
}

void Guild::removeInvited(int playerId)
{
    mInvited.remove(playerId);
}

bool Guild::checkInGuild(int playerId) const
{
    return getMember(playerId) != 0;
}

GuildMember *Guild::getMember(int playerId) const
{
    std::list<GuildMember*>::const_iterator itr = mMembers.begin(),
                                            itr_end = mMembers.end();
    while (itr != itr_end)
    {
        if ((*itr)->mId == playerId)
            return (*itr);
        ++itr;
    }

    return 0;
}

bool Guild::canInvite(int playerId) const
{
    // Guild members with permissions above NONE can invite
    // Check that guild members permissions are not NONE
    GuildMember *member = getMember(playerId);
    if (member->mPermissions & GAL_INVITE)
        return true;
    return false;
}

int Guild::getUserPermissions(int playerId) const
{
    GuildMember *member = getMember(playerId);
    return member ? member->mPermissions : 0;
}

void Guild::setUserPermissions(int playerId, int level)
{
    GuildMember *member = getMember(playerId);
    member->mPermissions = level;
}
