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
#include "chatclient.h"
#include "party.h"

#include "account-server/storage.h"
#include "account-server/serverhandler.h"

#include "common/manaserv_protocol.h"

#include "net/messagein.h"
#include "net/messageout.h"

using namespace ManaServ;

void updateInfo(ChatClient *client, int partyId)
{
    CharacterData *character = storage->getCharacter(client->characterName);
    GameServerHandler::sendPartyChange(character, partyId);
}

void ChatHandler::removeExpiredPartyInvites()
{
    time_t now = time(nullptr);
    while (!mInvitations.empty() && mInvitations.front().mExpireTime < now)
    {
        std::map<std::string, int>::iterator itr;
        itr = mNumInvites.find(mInvitations.front().mInviter);
        if (--itr->second <= 0)
            mNumInvites.erase(itr);
        mInvitations.pop_front();
    }
}

void ChatHandler::handlePartyInvite(MessageIn &msg)
{
    std::string inviterName = msg.readString();
    std::string inviteeName = msg.readString();
    ChatClient *inviter = getClient(inviterName);
    ChatClient *invitee = getClient(inviteeName);

    if (!inviter || !invitee)
        return;

    removeExpiredPartyInvites();
    const int maxInvitesPerTimeframe = 10;
    int &num = mNumInvites[inviterName];
    if (num >= maxInvitesPerTimeframe)
    {
        MessageOut out(CPMSG_PARTY_REJECTED);
        out.writeString(inviterName);
        out.writeInt8(ERRMSG_LIMIT_REACHED);
        inviter->send(out);
        return;
    }
    ++num;

    if (invitee->party)
    {
        MessageOut out(CPMSG_PARTY_REJECTED);
        out.writeString(inviterName);
        out.writeInt8(ERRMSG_FAILURE);
        inviter->send(out);
        return;
    }

    mInvitations.push_back(PartyInvite(inviterName, inviteeName));

    MessageOut out(CPMSG_PARTY_INVITED);
    out.writeString(inviterName);
    invitee->send(out);
}

void ChatHandler::handlePartyInviteAnswer(ChatClient &client, MessageIn &msg)
{
    if (client.party)
        return;

    MessageOut outInvitee(CPMSG_PARTY_INVITE_ANSWER_RESPONSE);

    std::string inviter = msg.readString();

    // check if the invite is still valid
    bool valid = false;
    removeExpiredPartyInvites();
    const size_t size = mInvitations.size();
    for (size_t i = 0; i < size; ++i)
    {
        if (mInvitations[i].mInviter == inviter &&
            mInvitations[i].mInvitee == client.characterName)
        {
            valid = true;
        }
    }

    // the invitee did not accept the invitation
    if (!msg.readInt8())
    {
        if (!valid)
            return;

        // send rejection to inviter
        ChatClient *inviterClient = getClient(inviter);
        if (inviterClient)
        {
            MessageOut out(CPMSG_PARTY_REJECTED);
            out.writeString(inviter);
            out.writeInt8(ERRMSG_OK);
            inviterClient->send(out);
        }
        return;
    }

    // if the invitation has expired, tell the inivtee about it
    if (!valid)
    {
        outInvitee.writeInt8(ERRMSG_TIME_OUT);
        client.send(outInvitee);
        return;
    }

    // check that the inviter is still in the game
    ChatClient *c1 = getClient(inviter);
    if (!c1)
    {
        outInvitee.writeInt8(ERRMSG_FAILURE);
        client.send(outInvitee);
        return;
    }

    // if party doesnt exist, create it
    if (!c1->party)
    {
        c1->party = new Party();
        c1->party->addUser(inviter);
        // tell game server to update info
        updateInfo(c1, c1->party->getId());
    }

    outInvitee.writeInt8(ERRMSG_OK);
    Party::PartyUsers users = c1->party->getUsers();
    const unsigned usersSize = users.size();
    for (unsigned i = 0; i < usersSize; i++)
        outInvitee.writeString(users[i]);

    client.send(outInvitee);

    // add invitee to the party
    c1->party->addUser(client.characterName, inviter);
    client.party = c1->party;

    // tell game server to update info
    updateInfo(&client, client.party->getId());
}

void ChatHandler::handlePartyQuit(ChatClient &client)
{
    removeUserFromParty(client);
    MessageOut out(CPMSG_PARTY_QUIT_RESPONSE);
    out.writeInt8(ERRMSG_OK);
    client.send(out);

    // tell game server to update info
    updateInfo(&client, 0);
}

void ChatHandler::removeUserFromParty(ChatClient &client)
{
    if (client.party)
    {
        client.party->removeUser(client.characterName);
        informPartyMemberQuit(client);

        // if theres less than 1 member left, remove the party
        if (client.party->userCount() < 1)
        {
            delete client.party;
            client.party = 0;
        }
    }
}

void ChatHandler::informPartyMemberQuit(ChatClient &client)
{
    std::map<std::string, ChatClient*>::iterator itr;
    std::map<std::string, ChatClient*>::const_iterator itr_end = mPlayerMap.end();

    for (itr = mPlayerMap.begin(); itr != itr_end; ++itr)
    {
        if (itr->second->party == client.party)
        {
            MessageOut out(CPMSG_PARTY_MEMBER_LEFT);
            out.writeInt32(client.characterId);
            itr->second->send(out);
        }
    }
}
