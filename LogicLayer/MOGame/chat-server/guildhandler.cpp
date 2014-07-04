/*
 *  The Mana Server
 *  Copyright (C) 2008-2010  The Mana World Development Team
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

#include "chathandler.h"
#include "chatchannel.h"
#include "chatchannelmanager.h"
#include "chatclient.h"
#include "guild.h"
#include "guildmanager.h"

#include "account-server/character.h"
#include "account-server/storage.h"

#include "net/messagein.h"
#include "net/messageout.h"

#include "common/configuration.h"
#include "common/manaserv_protocol.h"

using namespace ManaServ;

void ChatHandler::sendGuildInvite(const std::string &invitedName,
                                  const std::string &inviterName,
                                  const std::string &guildName)
{
    MessageOut msg(CPMSG_GUILD_INVITED);
    msg.writeString(inviterName);
    msg.writeString(guildName);

    ChatClient *client = getClient(invitedName);
    if (client)
    {
        client->send(msg);
    }
}

void ChatHandler::sendGuildRejoin(ChatClient &client)
{
    // Get list of guilds and check what rights they have.
    std::vector<Guild *> guilds =
            guildManager->getGuildsForPlayer(client.characterId);

    client.guilds = guilds;

    for (std::vector<Guild *>::iterator it = guilds.begin(),
         it_end = guilds.end(); it != it_end; ++it)
    {
        Guild *guild = *it;

        const int permissions = guild->getUserPermissions(client.characterId);
        const std::string guildName = guild->getName();

        // Tell the client what guilds the character belongs to
        // and their permissions
        MessageOut msg(CPMSG_GUILD_REJOIN);
        msg.writeString(guildName);
        msg.writeInt16(guild->getId());
        msg.writeInt16(permissions);

        // get channel id of guild channel
        ChatChannel *channel = joinGuildChannel(guildName, client);

        // send the channel id for the autojoined channel
        msg.writeInt16(channel->getId());
        msg.writeString(channel->getAnnouncement());

        client.send(msg);

        sendGuildListUpdate(guild, client.characterName,
                            GUILD_EVENT_ONLINE_PLAYER);
    }
}

ChatChannel *ChatHandler::joinGuildChannel(const std::string &guildName, ChatClient &client)
{
    // Automatically make the character join the guild chat channel
    ChatChannel *channel = chatChannelManager->getChannel(guildName);
    if (!channel)
    {
        // Channel doesnt exist so create it
        int channelId = chatChannelManager->createNewChannel(
                    guildName, "Guild Channel", std::string(), false);
        channel = chatChannelManager->getChannel(channelId);
    }

    // Add user to the channel
    if (channel->addUser(&client))
    {
        // Send an CPMSG_UPDATE_CHANNEL to warn other clients a user went
        // in the channel.
        warnUsersAboutPlayerEventInChat(channel, client.characterName,
                CHAT_EVENT_NEW_PLAYER);
    }

    return channel;
}

void ChatHandler::sendGuildListUpdate(Guild *guild,
                                      const std::string &characterName,
                                      char eventId)
{
    MessageOut msg(CPMSG_GUILD_UPDATE_LIST);

    msg.writeInt16(guild->getId());
    msg.writeString(characterName);
    msg.writeInt8(eventId);
    std::map<std::string, ChatClient*>::const_iterator chr;
    std::list<GuildMember*> members = guild->getMembers();

    for (std::list<GuildMember*>::const_iterator itr = members.begin();
         itr != members.end(); ++itr)
    {
        CharacterData *c = storage->getCharacter((*itr)->mId, nullptr);
        chr = mPlayerMap.find(c->getName());
        if (chr != mPlayerMap.end())
        {
            chr->second->send(msg);
        }
    }
}

void ChatHandler::handleGuildCreate(ChatClient &client, MessageIn &msg)
{
    MessageOut reply(CPMSG_GUILD_CREATE_RESPONSE);

    // Check if guild already exists and if so, return error
    std::string guildName = msg.readString();
    if (!guildManager->doesExist(guildName))
    {
        if ((int)client.guilds.size() >=
                Configuration::getValue("account_maxGuildsPerCharacter", 1))
        {
            reply.writeInt8(ERRMSG_LIMIT_REACHED);
        }
        else
        {
            // Guild doesnt already exist so create it
            Guild *guild = guildManager->createGuild(guildName, client.characterId);
            reply.writeInt8(ERRMSG_OK);
            reply.writeString(guildName);
            reply.writeInt16(guild->getId());
            reply.writeInt16(guild->getUserPermissions(client.characterId));

            client.guilds.push_back(guild);

            // Send autocreated channel id
            ChatChannel* channel = joinGuildChannel(guildName, client);
            reply.writeInt16(channel->getId());
        }
    }
    else
    {
        reply.writeInt8(ERRMSG_ALREADY_TAKEN);
    }

    client.send(reply);
}

void ChatHandler::handleGuildInvite(ChatClient &client, MessageIn &msg)
{
    MessageOut reply(CPMSG_GUILD_INVITE_RESPONSE);
    MessageOut invite(CPMSG_GUILD_INVITED);

    // send an invitation from sender to character to join guild
    int guildId = msg.readInt16();
    std::string character = msg.readString();

    // get the chat client and the guild
    ChatClient *invitedClient = getClient(character);
    Guild *guild = guildManager->findById(guildId);

    if (invitedClient && guild)
    {
        // check permissions of inviter, and that they arent inviting themself,
        // and arent someone already in the guild
        if (guild->canInvite(client.characterId) &&
            client.characterName != character &&
            guild->checkInGuild(client.characterId))
        {
            if ((int)invitedClient->guilds.size() >=
                    Configuration::getValue("account_maxGuildsPerCharacter", 1))
            {
                reply.writeInt8(ERRMSG_LIMIT_REACHED);
            }
            else if (guild->checkInGuild(invitedClient->characterId))
            {
                reply.writeInt8(ERRMSG_ALREADY_MEMBER);
            }
            else
            {
                // send the name of the inviter and the name of the guild
                // that the character has been invited to join
                std::string senderName = client.characterName;
                std::string guildName = guild->getName();
                invite.writeString(senderName);
                invite.writeString(guildName);
                invite.writeInt16(guildId);
                invitedClient->send(invite);
                reply.writeInt8(ERRMSG_OK);

                // add member to list of invited members to the guild
                guild->addInvited(invitedClient->characterId);
            }
        }
        else
        {
            reply.writeInt8(ERRMSG_FAILURE);
        }
    }
    else
    {
        reply.writeInt8(ERRMSG_FAILURE);
    }

    client.send(reply);
}

void ChatHandler::handleGuildAcceptInvite(ChatClient &client,
                                          MessageIn &msg)
{
    MessageOut reply(CPMSG_GUILD_ACCEPT_RESPONSE);
    const int guildId = msg.readInt16();
    const bool accepted = msg.readInt8();

    // check guild exists and that member was invited
    // then add them as guild member
    // and remove from invite list
    Guild *guild = guildManager->findById(guildId);
    if (!(guild && guild->checkInvited(client.characterId)))
    {

        reply.writeInt8(ERRMSG_FAILURE);
    }
    else if (accepted)
    {
        // add user to guild
        guildManager->addGuildMember(guild, client.characterId);
        client.guilds.push_back(guild);
        reply.writeInt8(ERRMSG_OK);
        reply.writeString(guild->getName());
        reply.writeInt16(guild->getId());
        reply.writeInt16(guild->getUserPermissions(client.characterId));

        // have character join guild channel
        ChatChannel *channel = joinGuildChannel(guild->getName(), client);
        reply.writeInt16(channel->getId());
        sendGuildListUpdate(guild, client.characterName,
                            GUILD_EVENT_NEW_PLAYER);
    }
    else
    {
        guild->removeInvited(client.characterId);
        reply.writeInt8(ERRMSG_OK);
    }

    client.send(reply);
}

void ChatHandler::handleGuildGetMembers(ChatClient &client, MessageIn &msg)
{
    MessageOut reply(CPMSG_GUILD_GET_MEMBERS_RESPONSE);
    short guildId = msg.readInt16();
    Guild *guild = guildManager->findById(guildId);

    // check for valid guild
    // write a list of member names that belong to the guild
    if (guild)
    {
        // make sure the requestor is in the guild
        if (guild->checkInGuild(client.characterId))
        {
            reply.writeInt8(ERRMSG_OK);
            reply.writeInt16(guildId);
            std::list<GuildMember*> memberList = guild->getMembers();
            std::list<GuildMember*>::const_iterator itr_end = memberList.end();
            for (std::list<GuildMember*>::iterator itr = memberList.begin();
                 itr != itr_end; ++itr)
            {
                CharacterData *c = storage->getCharacter((*itr)->mId, nullptr);
                std::string memberName = c->getName();
                reply.writeString(memberName);
                reply.writeInt8(mPlayerMap.find(memberName) != mPlayerMap.end());
            }
        }
    }
    else
    {
        reply.writeInt8(ERRMSG_FAILURE);
    }

    client.send(reply);
}

void ChatHandler::handleGuildMemberLevelChange(ChatClient &client,
                                               MessageIn &msg)
{
    // get the guild, the user to change the permissions, and the new permission
    // check theyre valid, and then change them
    MessageOut reply(CPMSG_GUILD_PROMOTE_MEMBER_RESPONSE);
    short guildId = msg.readInt16();
    std::string user = msg.readString();
    short level = msg.readInt8();
    Guild *guild = guildManager->findById(guildId);
    CharacterData *c = storage->getCharacter(user);

    if (guild && c)
    {
        int rights = guild->getUserPermissions(c->getDatabaseID()) | level;
        if (guildManager->changeMemberLevel(&client, guild, c->getDatabaseID(),
                                            rights) == 0)
        {
            reply.writeInt8(ERRMSG_OK);
            client.send(reply);
        }
    }

    reply.writeInt8(ERRMSG_FAILURE);
    client.send(reply);
}

void ChatHandler::handleGuildKickMember(ChatClient &client, MessageIn &msg)
{
    MessageOut reply(CPMSG_GUILD_KICK_MEMBER_RESPONSE);
    short guildId = msg.readInt16();
    std::string otherCharName = msg.readString();

    Guild *guild = guildManager->findById(guildId);

    if (!guild)
    {
        reply.writeInt8(ERRMSG_INVALID_ARGUMENT);
        client.send(reply);
        return;
    }
    ChatClient *otherClient = getClient(otherCharName);
    unsigned otherCharId;
    if (otherClient)
        otherCharId = otherClient->characterId;
    else
        otherCharId = storage->getCharacterId(otherCharName);

    if (otherCharId == 0)
    {
        reply.writeInt8(ERRMSG_INVALID_ARGUMENT);
        client.send(reply);
        return;
    }

    if (!((guild->getUserPermissions(client.characterId) & GAL_KICK) &&
            guild->checkInGuild(otherCharId) &&
            otherCharId != client.characterId))
    {
        reply.writeInt8(ERRMSG_INSUFFICIENT_RIGHTS);
        client.send(reply);
        return;
    }
    if (otherClient)
    {
        // Client is online. Inform him about that he got kicked
        MessageOut kickMsg(CPMSG_GUILD_KICK_NOTIFICATION);
        kickMsg.writeInt16(guild->getId());
        kickMsg.writeString(client.characterName);
        otherClient->send(kickMsg);
    }

    guildManager->removeGuildMember(guild, otherCharId, otherCharName,
                                    otherClient);
    reply.writeInt8(ERRMSG_OK);
    client.send(reply);
}

void ChatHandler::handleGuildQuit(ChatClient &client, MessageIn &msg)
{
    MessageOut reply(CPMSG_GUILD_QUIT_RESPONSE);
    short guildId = msg.readInt16();

    Guild *guild = guildManager->findById(guildId);
    if (!guild || !guild->checkInGuild(client.characterId))
    {
        reply.writeInt8(ERRMSG_FAILURE);
        client.send(reply);
        return;
    }
    guildManager->removeGuildMember(guild, client.characterId,
                                    client.characterName, &client);

    reply.writeInt8(ERRMSG_OK);
    reply.writeInt16(guildId);
    client.send(reply);


}

void ChatHandler::guildChannelTopicChange(ChatChannel *channel, int playerId,
                                          const std::string &topic)
{
    Guild *guild = guildManager->findByName(channel->getName());
    if (guild && guild->getUserPermissions(playerId) & GAL_TOPIC_CHANGE)
    {
        chatChannelManager->setChannelTopic(channel->getId(), topic);
    }
}
