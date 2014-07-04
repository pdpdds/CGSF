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

#include <cassert>
#include <map>

#include "gamehandler.h"

#include "common/configuration.h"
#include "common/transaction.h"
#include "accountconnection.h"
#include "buysell.h"
#include "commandhandler.h"
#include "emotemanager.h"
#include "inventory.h"
#include "item.h"
#include "itemmanager.h"
#include "map.h"
#include "mapcomposite.h"
#include "npc.h"
#include "postman.h"
#include "state.h"
#include "trade.h"
#include "net/messagein.h"
#include "net/messageout.h"
#include "net/netcomputer.h"
#include "utils/logger.h"
#include "utils/tokendispenser.h"

#pragma comment(lib, "physfs.lib")

const unsigned TILES_TO_BE_NEAR = 7;

GameHandler::GameHandler():
    mTokenCollector(this)
{
}

bool GameHandler::startListen(enet_uint16 port)
{
    LOG_INFO("Game handler started:");
    return ConnectionHandler::startListen(port);
}

NetComputer *GameHandler::computerConnected(ENetPeer *peer)
{
    return new GameClient(peer);
}

void GameHandler::computerDisconnected(NetComputer *comp)
{
    GameClient &computer = *static_cast< GameClient * >(comp);

    if (computer.status == CLIENT_QUEUED)
    {
        mTokenCollector.deletePendingClient(&computer);
    }
    else if (Entity *ch = computer.character)
    {
        accountHandler->sendCharacterData(ch);
        ch->getComponent<CharacterComponent>()->disconnected(*ch);
        delete ch;
    }
    delete &computer;
}

void GameHandler::detachClient(Entity *ch)
{
    auto *component = ch->getComponent<CharacterComponent>();
    GameClient *client = component->getClient();
    assert(client);
    client->character = nullptr;
    client->status = CLIENT_LOGIN;
    component->setClient(nullptr);
}

void GameHandler::prepareServerChange(Entity *ch)
{
    GameClient *client = ch->getComponent<CharacterComponent>()->getClient();
    assert(client);
    client->status = CLIENT_CHANGE_SERVER;
}

void GameHandler::completeServerChange(int id, const std::string &token,
                                       const std::string &address, int port)
{
    for (NetComputers::const_iterator i = clients.begin(),
         i_end = clients.end(); i != i_end; ++i)
    {
        GameClient *c = static_cast< GameClient * >(*i);
        if (c->status == CLIENT_CHANGE_SERVER &&
            c->character->getComponent<CharacterComponent>()
                ->getDatabaseID() == id)
        {
            MessageOut msg(GPMSG_PLAYER_SERVER_CHANGE);
            msg.writeString(token, MAGIC_TOKEN_LENGTH);
            msg.writeString(address);
            msg.writeInt16(port);
            c->send(msg);
            c->character->getComponent<CharacterComponent>()->disconnected(
                    *c->character);
            delete c->character;
            c->character = nullptr;
            c->status = CLIENT_LOGIN;
            return;
        }
    }
}

void GameHandler::updateCharacter(int charid, int partyid)
{
    for (NetComputers::const_iterator i = clients.begin(),
         i_end = clients.end(); i != i_end; ++i)
    {
        GameClient *c = static_cast< GameClient * >(*i);
        auto *characterComponent =
                c->character->getComponent<CharacterComponent>();

        if (characterComponent->getDatabaseID() == charid)
            characterComponent->setParty(partyid);
    }
}

static Entity *findActorNear(Entity *p, int id)
{
    MapComposite *map = p->getMap();

    if (Entity *e = map->findEntityById(id)) {
        const Point &ppos = p->getComponent<ActorComponent>()->getPosition();
        const Point &epos = e->getComponent<ActorComponent>()->getPosition();

        // See map.h for tiles constants
        const int pixelDist = DEFAULT_TILE_LENGTH * TILES_TO_BE_NEAR;
        if (ppos.inRangeOf(epos, pixelDist))
            return e;
    }

    return 0;
}

static Entity *findBeingNear(Entity *p, int id)
{
    if (Entity *e = findActorNear(p, id))
        if (e->hasComponent<BeingComponent>())
            return e;

    return 0;
}

static Entity *findCharacterNear(Entity *p, int id)
{
    if (Entity *e = findActorNear(p, id))
        if (e->getType() == OBJECT_CHARACTER)
            return e;

    return 0;
}

void GameHandler::processMessage(NetComputer *computer, MessageIn &message)
{
    GameClient &client = *static_cast<GameClient *>(computer);

    if (client.status == CLIENT_LOGIN)
    {
        if (message.getId() != PGMSG_CONNECT)
            return;

        std::string magic_token = message.readString(MAGIC_TOKEN_LENGTH);
        client.status = CLIENT_QUEUED; // Before the addPendingClient
        mTokenCollector.addPendingClient(magic_token, &client);
        return;
    }
    else if (client.status != CLIENT_CONNECTED)
    {
        return;
    }

    switch (message.getId())
    {
        case PGMSG_SAY:
            handleSay(client, message);
            break;

        case PGMSG_NPC_TALK:
        case PGMSG_NPC_TALK_NEXT:
        case PGMSG_NPC_SELECT:
        case PGMSG_NPC_NUMBER:
        case PGMSG_NPC_STRING:
            handleNpc(client, message);
            break;

        case PGMSG_PICKUP:
            handlePickup(client, message);
            break;

        case PGMSG_USE_ITEM:
            handleUseItem(client, message);
            break;

        case PGMSG_DROP:
            handleDrop(client, message);
            break;

        case PGMSG_WALK:
            handleWalk(client, message);
            break;

        case PGMSG_EQUIP:
            handleEquip(client, message);
            break;

        case PGMSG_UNEQUIP:
            handleUnequip(client, message);
            break;

        case PGMSG_USE_ABILITY_ON_BEING:
            handleUseAbilityOnBeing(client, message);
            break;

        case PGMSG_USE_ABILITY_ON_POINT:
            handleUseAbilityOnPoint(client, message);
            break;

        case PGMSG_USE_ABILITY_ON_DIRECTION:
            handleUseAbilityOnDirection(client, message);
            break;

        case PGMSG_ACTION_CHANGE:
            handleActionChange(client, message);
            break;

        case PGMSG_DIRECTION_CHANGE:
            handleDirectionChange(client, message);
            break;

        case PGMSG_DISCONNECT:
            handleDisconnect(client, message);
            break;

        case PGMSG_TRADE_REQUEST:
            handleTradeRequest(client, message);
            break;

        case PGMSG_TRADE_CANCEL:
        case PGMSG_TRADE_AGREED:
        case PGMSG_TRADE_CONFIRM:
        case PGMSG_TRADE_ADD_ITEM:
        case PGMSG_TRADE_SET_MONEY:
            handleTrade(client, message);
            break;

        case PGMSG_NPC_BUYSELL:
            handleNpcBuySell(client, message);
            break;

        case PGMSG_RAISE_ATTRIBUTE:
            handleRaiseAttribute(client, message);
            break;

        case PGMSG_LOWER_ATTRIBUTE:
            handleLowerAttribute(client, message);
            break;

        case PGMSG_RESPAWN:
            // plausibility check is done by character class
            client.character->getComponent<CharacterComponent>()->respawn(
                    *client.character);
            break;

        case PGMSG_NPC_POST_SEND:
            handleNpcPostSend(client, message);
            break;

        case PGMSG_PARTY_INVITE:
            handlePartyInvite(client, message);
            break;

        case PGMSG_BEING_EMOTE:
            handleTriggerEmoticon(client, message);
            break;

        default:
            LOG_WARN("Invalid message type");
            client.send(MessageOut(XXMSG_INVALID));
            break;
    }
}

void GameHandler::sendTo(Entity *beingPtr, MessageOut &msg)
{
    GameClient *client = beingPtr->getComponent<CharacterComponent>()
            ->getClient();
    sendTo(client, msg);
}

void GameHandler::sendTo(GameClient *client, MessageOut &msg)
{
    assert(client && client->status == CLIENT_CONNECTED);
    client->send(msg);
}

void GameHandler::addPendingCharacter(const std::string &token, Entity *ch)
{
    /* First, check if the character is already on the map. This may happen if
       a client just lost its connection, and logged to the account server
       again, yet the game server has not yet detected the lost connection. */

    int id = ch->getComponent<CharacterComponent>()->getDatabaseID();

    for (NetComputer *i : clients)
    {
        GameClient *c = static_cast< GameClient * >(i);
        Entity *old_ch = c->character;
        if (old_ch && id == old_ch->getComponent<CharacterComponent>()->getDatabaseID())
        {
            if (c->status != CLIENT_CONNECTED)
            {
                /* Either the server is confused, or the client is up to no
                   good. So ignore the request, and wait for the connections
                   to properly time out. */
                return;
            }

            /* As the connection was not properly closed, the account server
               has not yet updated its data, so ignore them. Instead, take the
               already present character, kill its current connection, and make
               it available for a new connection. */
            delete ch;

            GameState::remove(old_ch);
            detachClient(old_ch);
            MessageOut msg(GPMSG_CONNECT_RESPONSE);
            msg.writeInt8(ERRMSG_LOGIN_WAS_TAKEN_OVER);
            c->disconnect(msg);

            ch = old_ch;
            break;
        }
    }

    // Mark the character as pending a connection.
    mTokenCollector.addPendingConnect(token, ch);
}

void GameHandler::tokenMatched(GameClient *computer, Entity *character)
{
    computer->character = character;
    computer->status = CLIENT_CONNECTED;

    auto *characterComponent =
            character->getComponent<CharacterComponent>();

    characterComponent->setClient(computer);

    MessageOut result(GPMSG_CONNECT_RESPONSE);

    if (!GameState::insert(character))
    {
        result.writeInt8(ERRMSG_SERVER_FULL);
        detachClient(character);
        delete character;
        computer->disconnect(result);
        return;
    }
    // Trigger login script bind
    characterComponent->triggerLoginCallback(*character);

    result.writeInt8(ERRMSG_OK);
    computer->send(result);

    Inventory(character).sendFull();
    characterComponent->markAllInfoAsChanged(*character);
}

void GameHandler::deletePendingClient(GameClient *computer)
{
    // Something might have changed since it was inserted
    if (computer->status != CLIENT_QUEUED)
        return;

    MessageOut msg(GPMSG_CONNECT_RESPONSE);
    msg.writeInt8(ERRMSG_TIME_OUT);

    // The computer will be deleted when the disconnect event is processed
    computer->disconnect(msg);
}

void GameHandler::deletePendingConnect(Entity *character)
{
    delete character;
}

Entity *GameHandler::getCharacterByNameSlow(const std::string &name) const
{
    for (NetComputers::const_iterator i = clients.begin(),
         i_end = clients.end(); i != i_end; ++i)
    {
        GameClient *c = static_cast< GameClient * >(*i);
        Entity *ch = c->character;
        if (ch && ch->getComponent<BeingComponent>()->getName() == name &&
                c->status == CLIENT_CONNECTED)
        {
            return ch;
        }
    }
    return 0;
}

void GameHandler::handleSay(GameClient &client, MessageIn &message)
{
    const std::string say = message.readString();
    if (say.empty())
        return;

    if (say[0] == '@')
    {
        CommandHandler::handleCommand(client.character, say);
        return;
    }
    if (!client.character->getComponent<CharacterComponent>()->isMuted())
    {
        GameState::sayAround(client.character, say);
    }
    else
    {
        GameState::sayTo(client.character, nullptr,
                         "You are not allowed to talk right now.");
    }
}

void GameHandler::handleNpc(GameClient &client, MessageIn &message)
{
    int id = message.readInt16();
    Entity *actor = findActorNear(client.character, id);
    if (!actor || actor->getType() != OBJECT_NPC)
    {
        sendNpcError(client, id, "Not close enough to NPC\n");
        return;
    }
    switch (message.getId())
    {
        case PGMSG_NPC_SELECT:
            Npc::integerReceived(client.character, message.readInt8());
            break;
        case PGMSG_NPC_NUMBER:
            Npc::integerReceived(client.character, message.readInt32());
            break;
        case PGMSG_NPC_STRING:
            Npc::stringReceived(client.character, message.readString());
            break;
        case PGMSG_NPC_TALK:
            Npc::start(actor, client.character);
            break;
        case PGMSG_NPC_TALK_NEXT:
        default:
            Npc::resume(client.character);
            break;
    }
}

void GameHandler::handlePickup(GameClient &client, MessageIn &message)
{
    const int x = message.readInt16();
    const int y = message.readInt16();
    const Point ppos =
            client.character->getComponent<ActorComponent>()->getPosition();

    // TODO: use a less arbitrary value.
    if (std::abs(x - ppos.x) + std::abs(y - ppos.y) < 48)
    {
        MapComposite *map = client.character->getMap();
        Point ipos(x, y);
        for (FixedActorIterator i(map->getAroundPointIterator(ipos, 0)); i; ++i)
        {
            Entity *o = *i;
            Point opos = o->getComponent<ActorComponent>()->getPosition();

            if (o->getType() == OBJECT_ITEM && opos.x == x && opos.y == y)
            {
                ItemComponent *item = o->getComponent<ItemComponent>();
                ItemClass *ic = item->getItemClass();
                int amount = item->getAmount();

                if (!Inventory(client.character).insert(ic->getDatabaseID(),
                                                        amount))
                {
                    GameState::remove(o);

                    // We only do this when items are to be kept in memory
                    // between two server restart.
                    if (!Configuration::getValue("game_floorItemDecayTime", 0))
                    {
                        // Remove the floor item from map
                        accountHandler->removeFloorItems(map->getID(),
                                                         ic->getDatabaseID(),
                                                         amount, x, y);
                    }

                    // log transaction
                    std::stringstream str;
                    str << "User picked up item " << ic->getDatabaseID()
                        << " at " << opos.x << "x" << opos.y;
                    auto *characterComponent = client.character
                            ->getComponent<CharacterComponent>();
                    accountHandler->sendTransaction(
                            characterComponent->getDatabaseID(),
                            TRANS_ITEM_PICKUP, str.str()
                                                   );
                }
                break;
            }
        }
    }
}

void GameHandler::handleUseItem(GameClient &client, MessageIn &message)
{
    if (client.character->getComponent<BeingComponent>()->getAction() == DEAD)
        return;

    const int slot = message.readInt16();

    Inventory inv(client.character);

    if (ItemClass *ic = itemManager->getItem(inv.getItem(slot)))
    {
        if (ic->hasTrigger(ITT_ACTIVATE))
        {
            std::stringstream str;
            str << "User activated item " << ic->getDatabaseID()
                << " from slot " << slot;
            auto *characterComponent = client.character
                    ->getComponent<CharacterComponent>();
            accountHandler->sendTransaction(characterComponent->getDatabaseID(),
                                            TRANS_ITEM_USED, str.str());
            if (ic->useTrigger(client.character, ITT_ACTIVATE))
                inv.removeFromSlot(slot, 1);
        }
    }
}

void GameHandler::handleDrop(GameClient &client, MessageIn &message)
{
    const int slot = message.readInt16();
    const int amount = message.readInt16();
    Inventory inv(client.character);

    if (ItemClass *ic = itemManager->getItem(inv.getItem(slot)))
    {
        int nb = inv.removeFromSlot(slot, amount);
        MapComposite *map = client.character->getMap();
        const Point &pos = client.character->getComponent<ActorComponent>()
                ->getPosition();

        Entity *item = Item::create(map, pos, ic, amount - nb);

        if (!GameState::insertOrDelete(item))
        {
            // The map is full. Put back into inventory.
            inv.insert(ic->getDatabaseID(), amount - nb);
            return;
        }

        // We store the item in database only when the floor items are meant
        // to be persistent between two server restarts.
        if (!Configuration::getValue("game_floorItemDecayTime", 0))
        {
            // Create the floor item on map
            accountHandler->createFloorItems(client.character->getMap()->getID(),
                                             ic->getDatabaseID(),
                                             amount, pos.x, pos.y);
        }

        // log transaction
        std::stringstream str;
        str << "User dropped item " << ic->getDatabaseID()
            << " at " << pos.x << "x" << pos.y;
        auto *characterComponent = client.character
                ->getComponent<CharacterComponent>();
        accountHandler->sendTransaction(characterComponent->getDatabaseID(),
                                        TRANS_ITEM_DROP, str.str());
    }
}

void GameHandler::handleWalk(GameClient &client, MessageIn &message)
{
    const int x = message.readInt16();
    const int y = message.readInt16();

    Point dst(x, y);
    client.character->getComponent<BeingComponent>()->setDestination(
            *client.character, dst);
}

void GameHandler::handleEquip(GameClient &client, MessageIn &message)
{
    const int slot = message.readInt16();
    MessageOut msg(GPMSG_EQUIP_RESPONSE);
    if (Inventory(client.character).equip(slot))
        msg.writeInt8(ERRMSG_OK);
    else
        msg.writeInt8(ERRMSG_FAILURE);
    msg.writeInt16(slot);
    client.send(msg);
}

void GameHandler::handleUnequip(GameClient &client, MessageIn &message)
{
    const int slot = message.readInt16();
    MessageOut msg(GPMSG_UNEQUIP_RESPONSE);
    if (Inventory(client.character).unequip(slot))
        msg.writeInt8(ERRMSG_OK);
    else
        msg.writeInt8(ERRMSG_FAILURE);
    msg.writeInt16(slot);
    client.send(msg);
}

void GameHandler::handleUseAbilityOnBeing(GameClient &client, MessageIn &message)
{
    if (client.character->getComponent<BeingComponent>()->getAction() == DEAD)
        return;

    const int abilityID = message.readInt8();
    const int targetID = message.readInt16(); // 0 when no target is selected
    Entity *being = 0;
    if (targetID != 0)
        being = findBeingNear(client.character, targetID);

    const int publicId =
            client.character->getComponent<ActorComponent>()->getPublicID();
    LOG_DEBUG("Character " << publicId
              << " tries to use his ability " << abilityID);
    auto *abilityComponent = client.character
            ->getComponent<AbilityComponent>();
    abilityComponent->useAbilityOnBeing(*client.character, abilityID, being);
}

void GameHandler::handleUseAbilityOnPoint(GameClient &client, MessageIn &message)
{
    if (client.character->getComponent<BeingComponent>()->getAction() == DEAD)
        return;

    const int abilityID = message.readInt8();
    const int x = message.readInt16();
    const int y = message.readInt16();

    const int publicId =
            client.character->getComponent<ActorComponent>()->getPublicID();
    LOG_DEBUG("Character " << publicId
              << " tries to use his ability " << abilityID);
    auto *abilityComponent = client.character
            ->getComponent<AbilityComponent>();
    abilityComponent->useAbilityOnPoint(*client.character, abilityID, x, y);
}

void GameHandler::handleUseAbilityOnDirection(GameClient &client, MessageIn &message)
{
    if (client.character->getComponent<BeingComponent>()->getAction() == DEAD)
        return;

    const int abilityID = message.readInt8();
    const BeingDirection direction = (BeingDirection)message.readInt8();

    const int publicId =
            client.character->getComponent<ActorComponent>()->getPublicID();
    LOG_DEBUG("Character " << publicId
              << " tries to use his ability " << abilityID);
    auto *abilityComponent = client.character
            ->getComponent<AbilityComponent>();
    abilityComponent->useAbilityOnDirection(*client.character, abilityID,
                                            direction);
}

void GameHandler::handleActionChange(GameClient &client, MessageIn &message)
{
    auto *beingComponent = client.character->getComponent<BeingComponent>();

    const BeingAction action = (BeingAction) message.readInt8();
    const BeingAction current = (BeingAction) beingComponent->getAction();
    bool logActionChange = true;

    switch (action)
    {
        case STAND:
            if (current == SIT)
            {
                beingComponent->setAction(*client.character, STAND);
                logActionChange = false;
            }
            break;
        case SIT:
            if (current == STAND)
            {
                beingComponent->setAction(*client.character, SIT);
                logActionChange = false;
            }
            break;
        default:
            break;
    }

    // Log the action change only when this is relevant.
    if (logActionChange)
    {
        // log transaction
        std::stringstream str;
        str << "User changed action from " << current << " to " << action;

        auto *characterComponent =
                client.character->getComponent<CharacterComponent>();

        accountHandler->sendTransaction(characterComponent->getDatabaseID(),
                                        TRANS_ACTION_CHANGE, str.str());
    }

}

void GameHandler::handleDirectionChange(GameClient &client, MessageIn &message)
{
    const BeingDirection direction = (BeingDirection) message.readInt8();
    client.character->getComponent<BeingComponent>()
            ->setDirection(*client.character, direction);
}

void GameHandler::handleDisconnect(GameClient &client, MessageIn &message)
{
    const bool reconnectAccount = (bool) message.readInt8();

    MessageOut result(GPMSG_DISCONNECT_RESPONSE);
    result.writeInt8(ERRMSG_OK); // It is, when control reaches here

    auto *characterComponent =
            client.character->getComponent<CharacterComponent>();

    if (reconnectAccount)
    {
        std::string magic_token(utils::getMagicToken());
        result.writeString(magic_token, MAGIC_TOKEN_LENGTH);
        // No accountserver data, the client should remember that
        accountHandler->playerReconnectAccount(
                    characterComponent->getDatabaseID(),
                    magic_token);
    }
    accountHandler->sendCharacterData(client.character);

    characterComponent->disconnected(*client.character);
    delete client.character;
    client.character = 0;
    client.status = CLIENT_LOGIN;

    client.send(result);
}

void GameHandler::handleTradeRequest(GameClient &client, MessageIn &message)
{
    const int id = message.readInt16();

    auto *characterComponent =
            client.character->getComponent<CharacterComponent>();

    if (Trade *t = characterComponent->getTrading())
        if (t->request(client.character, id))
            return;

    Entity *q = findCharacterNear(client.character, id);
    if (!q || characterComponent->isBusy())
    {
        client.send(MessageOut(GPMSG_TRADE_CANCEL));
        return;
    }

    new Trade(client.character, q);

    // log transaction
    std::string str;
    str = "User requested trade with " + q->getComponent<BeingComponent>()
            ->getName();
    accountHandler->sendTransaction(characterComponent->getDatabaseID(),
                                    TRANS_TRADE_REQUEST, str);
}

void GameHandler::handleTrade(GameClient &client, MessageIn &message)
{
    auto *characterComponent =
            client.character->getComponent<CharacterComponent>();

    int databaseId = characterComponent->getDatabaseID();

    std::stringstream str;
    Trade *t = characterComponent->getTrading();
    if (!t)
        return;

    switch (message.getId())
    {
        case PGMSG_TRADE_CANCEL:
            t->cancel();
            break;
        case PGMSG_TRADE_CONFIRM:
            t->confirm(client.character);
            break;
        case PGMSG_TRADE_AGREED:
            t->agree(client.character);
            // log transaction
            accountHandler->sendTransaction(databaseId,
                                            TRANS_TRADE_END,
                                            "User finished trading");
            break;
        case PGMSG_TRADE_SET_MONEY:
        {
            int money = message.readInt32();
            t->setMoney(client.character, money);
            // log transaction
            str << "User added " << money << " money to trade.";
            accountHandler->sendTransaction(databaseId,
                                            TRANS_TRADE_MONEY, str.str());
        } break;
        case PGMSG_TRADE_ADD_ITEM:
        {
            int slot = message.readInt8();
            t->addItem(client.character, slot, message.readInt8());
            // log transaction
            str << "User add item from slot " << slot;
            accountHandler->sendTransaction(databaseId,
                                            TRANS_TRADE_ITEM, str.str());
        } break;
    }
}

void GameHandler::handleNpcBuySell(GameClient &client, MessageIn &message)
{
    BuySell *t = client.character->getComponent<CharacterComponent>()
            ->getBuySell();
    if (!t)
        return;
    const int id = message.readInt16();
    const int amount = message.readInt16();
    t->perform(id, amount);
}

void GameHandler::handleRaiseAttribute(GameClient &client, MessageIn &message)
{
    auto *characterComponent =
            client.character->getComponent<CharacterComponent>();

    const int attributeId = message.readInt16();
    auto *attribute = attributeManager->getAttributeInfo(attributeId);
    AttribmodResponseCode retCode;
    if (attribute) {
        retCode = characterComponent->useCharacterPoint(*client.character,
                                                        attribute);
    } else {
        retCode = ATTRIBMOD_INVALID_ATTRIBUTE;
    }

    MessageOut result(GPMSG_RAISE_ATTRIBUTE_RESPONSE);
    result.writeInt8(retCode);
    result.writeInt16(attributeId);
    client.send(result);

    if (retCode == ATTRIBMOD_OK)
    {
        accountHandler->updateCharacterPoints(
                characterComponent->getDatabaseID(),
                characterComponent->getAttributePoints(),
                characterComponent->getCorrectionPoints());

        // log transaction
        std::stringstream str;
        str << "User increased attribute " << attribute;
        accountHandler->sendTransaction(characterComponent->getDatabaseID(),
                                        TRANS_ATTR_INCREASE, str.str());
    }
}

void GameHandler::handleLowerAttribute(GameClient &client, MessageIn &message)
{
    auto *characterComponent =
            client.character->getComponent<CharacterComponent>();

    const int attributeId = message.readInt32();
    auto *attribute = attributeManager->getAttributeInfo(attributeId);
    AttribmodResponseCode retCode;

    if (attribute) {
        retCode = characterComponent->useCorrectionPoint(*client.character,
                                                         attribute);
    } else {
        retCode = ATTRIBMOD_INVALID_ATTRIBUTE;
    }

    MessageOut result(GPMSG_LOWER_ATTRIBUTE_RESPONSE);
    result.writeInt8(retCode);
    result.writeInt16(attributeId);
    client.send(result);

    if (retCode == ATTRIBMOD_OK)
    {
        accountHandler->updateCharacterPoints(
                characterComponent->getDatabaseID(),
                characterComponent->getAttributePoints(),
                characterComponent->getCorrectionPoints());

        // log transaction
        std::stringstream str;
        str << "User decreased attribute " << attribute;
        accountHandler->sendTransaction(characterComponent->getDatabaseID(),
                                        TRANS_ATTR_DECREASE, str.str());
    }
}

void GameHandler::handleNpcPostSend(GameClient &client, MessageIn &message)
{
    // add the character so that the post man knows them
    postMan->addCharacter(client.character);
    accountHandler->sendPost(client.character, message);
}

void GameHandler::handlePartyInvite(GameClient &client, MessageIn &message)
{
    MapComposite *map = client.character->getMap();
    const int visualRange = Configuration::getValue("game_visualRange", 448);
    std::string invitee = message.readString();

    if (invitee == client.character->getComponent<BeingComponent>()->getName())
        return;

    for (CharacterIterator it(map->getWholeMapIterator()); it; ++it)
    {
        if ((*it)->getComponent<BeingComponent>()->getName() == invitee)
        {
            // calculate if the invitee is within the visual range
            auto *inviterComponent =
                    client.character->getComponent<ActorComponent>();
            auto *inviteeComponent = (*it)->getComponent<ActorComponent>();
            const Point &inviterPosition = inviterComponent->getPosition();
            const Point &inviteePosition = inviteeComponent->getPosition();
            const int dx = std::abs(inviterPosition.x - inviteePosition.x);
            const int dy = std::abs(inviterPosition.y - inviteePosition.y);
            if (visualRange > std::max(dx, dy))
            {
                MessageOut out(GCMSG_PARTY_INVITE);
                out.writeString(client.character
                                ->getComponent<BeingComponent>()->getName());
                out.writeString(invitee);
                accountHandler->send(out);
                return;
            }
            break;
        }
    }

    // Invitee was not found or is too far away
    MessageOut out(GPMSG_PARTY_INVITE_ERROR);
    out.writeString(invitee);
    client.send(out);
}

void GameHandler::handleTriggerEmoticon(GameClient &client, MessageIn &message)
{
    const int id = message.readInt16();
    if (emoteManager->isIdAvailable(id))
        client.character->getComponent<BeingComponent>()->triggerEmote(
                *client.character, id);
}

void GameHandler::sendNpcError(GameClient &client, int id,
                               const std::string &errorMsg)
{
    MessageOut msg(GPMSG_NPC_ERROR);
    msg.writeInt16(id);
    msg.writeString(errorMsg, errorMsg.size());
    client.send(msg);
}
