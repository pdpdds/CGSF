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

#include "guildmanager.h"
#include "guild.h"
#include "common/defines.h"
#include "common/manaserv_protocol.h"
#include "account-server/storage.h"
#include "chat-server/chatclient.h"
#include "chat-server/chatchannelmanager.h"
#include "chat-server/chathandler.h"

using namespace ManaServ;

GuildManager::GuildManager():
        mGuilds(storage->getGuildList())
{
}

GuildManager::~GuildManager()
{
    for (std::map<int, Guild*>::iterator it = mGuilds.begin();
            it != mGuilds.end(); ++it)
    {
        delete it->second;
    }
}

Guild* GuildManager::createGuild(const std::string &name, int playerId)
{
    Guild *guild = new Guild(name);
    // Add guild to db
    storage->addGuild(guild);

    // Add guild
    mGuilds[guild->getId()] = guild;

    // put the owner in the guild
    addGuildMember(guild, playerId);

    // Set and save the member rights
    storage->setMemberRights(guild->getId(), playerId, GAL_OWNER);

    guild->setOwner(playerId);

    return guild;
}

void GuildManager::removeGuild(Guild *guild)
{
    storage->removeGuild(guild);
    mGuilds.erase(guild->getId());
    delete guild;
}

void GuildManager::addGuildMember(Guild *guild, int playerId)
{
    storage->addGuildMember(guild->getId(), playerId);
    guild->addMember(playerId);
}

void GuildManager::removeGuildMember(Guild *guild, int playerId,
                                     const std::string &characterName,
                                     ChatClient *client)
{
    // remove the user from the guild
    storage->removeGuildMember(guild->getId(), playerId);
    guild->removeMember(playerId);

    chatHandler->sendGuildListUpdate(guild, characterName,
                                     GUILD_EVENT_LEAVING_PLAYER);

    // if theres no more members left delete the guild
    if (guild->memberCount() == 0)
    {
        chatChannelManager->removeChannel(
                    chatChannelManager->getChannelId(guild->getName()));
        removeGuild(guild);
    }

    if (client)
    {
        for (std::vector<Guild *>::iterator it = client->guilds.begin(),
             it_end = client->guilds.end(); it != it_end; ++it)
        {
            if (*it == guild)
            {
                client->guilds.erase(it);
                break;
            }
        }
    }
}

Guild *GuildManager::findById(short id) const
{
    std::map<int, Guild*>::const_iterator it = mGuilds.find(id);
    return it == mGuilds.end() ? 0 : it->second;
}

Guild *GuildManager::findByName(const std::string &name) const
{
    for (std::map<int, Guild*>::const_iterator it = mGuilds.begin(),
            it_end = mGuilds.end();
            it != it_end; ++it)
    {
        Guild *guild = it->second;
        if (guild->getName() == name)
            return guild;
    }
    return 0;
}

bool GuildManager::doesExist(const std::string &name) const
{
    return findByName(name) != 0;
}

std::vector<Guild *> GuildManager::getGuildsForPlayer(int playerId) const
{
    std::vector<Guild *> guilds;
    for (std::map<int, Guild*>::const_iterator it = mGuilds.begin();
         it != mGuilds.end(); ++it)
    {
        if (it->second->checkInGuild(playerId))
        {
            guilds.push_back(it->second);
        }
    }
    return guilds;
}

void GuildManager::disconnectPlayer(ChatClient *player)
{
    for (std::vector<Guild *>::iterator it = player->guilds.begin(),
         it_end = player->guilds.end(); it != it_end; ++it)
    {
        chatHandler->sendGuildListUpdate(*it,
                                         player->characterName,
                                         GUILD_EVENT_OFFLINE_PLAYER);
    }
}

int GuildManager::changeMemberLevel(ChatClient *player, Guild *guild,
                                    int playerId, int level)
{
    if (guild->checkInGuild(playerId))
    {
        int playerLevel = guild->getUserPermissions(player->characterId);

        if (playerLevel == GAL_OWNER)
        {
            // player can modify anyones permissions
            setUserRights(guild, playerId, level);
            return 0;
        }
    }

    return -1;
}

void GuildManager::setUserRights(Guild *guild, int playerId, int rights)
{
    // Set and save the member rights
    storage->setMemberRights(guild->getId(), playerId, rights);

    // Set with guild
    guild->setUserPermissions(playerId, rights);
}
