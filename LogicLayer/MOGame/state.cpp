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

#include "state.h"

#include "common/configuration.h"
#include "accountconnection.h"
#include "effect.h"
#include "gamehandler.h"
#include "inventory.h"
#include "item.h"
#include "itemmanager.h"
#include "map.h"
#include "mapcomposite.h"
#include "mapmanager.h"
#include "monster.h"
#include "npc.h"
#include "trade.h"
#include "net/messageout.h"
#include "scripting/script.h"
#include "scripting/scriptmanager.h"
#include "utils/logger.h"
#include "utils/speedconv.h"

#include <cassert>

enum
{
    EVENT_REMOVE = 0,
    EVENT_INSERT,
    EVENT_WARP
};

/**
 * Event expected to happen at next update.
 */
struct DelayedEvent
{
    unsigned short type;
    Point point;
    MapComposite *map;
};

typedef std::map< Entity *, DelayedEvent > DelayedEvents;

/**
 * The current world time in ticks since server start.
 */
static int currentTick;

/**
 * List of delayed events.
 */
static DelayedEvents delayedEvents;

/**
 * Cached persistent script variables
 */
static std::map< std::string, std::string > mScriptVariables;

/**
 * Sets message fields describing character look.
 */
static void serializeLooks(Entity *ch, MessageOut &msg)
{
    auto *characterComponent = ch->getComponent<CharacterComponent>();
    msg.writeInt8(characterComponent->getHairStyle());
    msg.writeInt8(characterComponent->getHairColor());
    const EquipData &equipData =
            characterComponent->getPossessions().getEquipment();
    const InventoryData &inventoryData =
            characterComponent->getPossessions().getInventory();

    // The map storing the info about the look changes to send
    //{ slot type id, item id }
    std::map<unsigned, unsigned> lookChanges;

    // Note that we can send several updates on the same slot type as different
    // items may have been equipped.
    for (EquipData::const_iterator it = equipData.begin(),
         it_end = equipData.end(); it != it_end; ++it)
    {
        InventoryData::const_iterator itemIt = inventoryData.find(*it);

        if (!itemManager->isEquipSlotVisible(itemIt->second.equipmentSlot))
            continue;

        lookChanges.insert(std::make_pair(
                itemIt->second.equipmentSlot,
                itemIt->second.itemId));
    }

    if (!lookChanges.empty())
    {
        // Number of look changes to send
        msg.writeInt8(lookChanges.size());

        for (std::map<unsigned, unsigned>::const_iterator it =
             lookChanges.begin(), it_end = lookChanges.end();
             it != it_end; ++it)
        {
            msg.writeInt8(it->first);
            msg.writeInt16(it->second);
        }
    }
}

/**
 * Informs a player of what happened around the character.
 */
static void informPlayer(MapComposite *map, Entity *p)
{
    MessageOut moveMsg(GPMSG_BEINGS_MOVE);
    MessageOut damageMsg(GPMSG_BEINGS_DAMAGE);
    const Point &pold = p->getComponent<BeingComponent>()->getOldPosition();
    const Point &ppos = p->getComponent<ActorComponent>()->getPosition();
    int pflags = p->getComponent<ActorComponent>()->getUpdateFlags();
    int visualRange = Configuration::getValue("game_visualRange", 448);

    // Inform client about activities of other beings near its character
    for (BeingIterator it(map->getAroundBeingIterator(p, visualRange));
         it; ++it)
    {
        Entity *o = *it;

        const Point &oold =
                o->getComponent<BeingComponent>()->getOldPosition();
        const Point &opos = o->getComponent<ActorComponent>()->getPosition();
        int otype = o->getType();
        int oid = o->getComponent<ActorComponent>()->getPublicID();
        int oflags = o->getComponent<ActorComponent>()->getUpdateFlags();
        int flags = 0;

        // Check if the character p and the moving object o are around.
        bool wereInRange = pold.inRangeOf(oold, visualRange) &&
                           !((pflags | oflags) & UPDATEFLAG_NEW_ON_MAP);
        bool willBeInRange = ppos.inRangeOf(opos, visualRange);

        if (!wereInRange && !willBeInRange)
        {
            // Nothing to report: o and p are far away from each other.
            continue;
        }


        if (wereInRange && willBeInRange)
        {
            // Send action change messages.
            if ((oflags & UPDATEFLAG_ACTIONCHANGE))
            {
                MessageOut actionMsg(GPMSG_BEING_ACTION_CHANGE);
                actionMsg.writeInt16(oid);
                actionMsg.writeInt8(
                        o->getComponent<BeingComponent>()->getAction());
                gameHandler->sendTo(p, actionMsg);
            }

            // Send looks change messages.
            if (oflags & UPDATEFLAG_LOOKSCHANGE)
            {
                MessageOut looksMsg(GPMSG_BEING_LOOKS_CHANGE);
                looksMsg.writeInt16(oid);
                serializeLooks(o, looksMsg);
                gameHandler->sendTo(p, looksMsg);
            }

            // Send emote messages.
            if (oflags & UPDATEFLAG_EMOTE)
            {
                int emoteId =
                        o->getComponent<BeingComponent>()->getLastEmote();
                if (emoteId > -1)
                {
                    MessageOut emoteMsg(GPMSG_BEING_EMOTE);
                    emoteMsg.writeInt16(oid);
                    emoteMsg.writeInt16(emoteId);
                    gameHandler->sendTo(p, emoteMsg);
                }
            }

            // Send direction change messages.
            if (oflags & UPDATEFLAG_DIRCHANGE && o != p)
            {
                MessageOut dirMsg(GPMSG_BEING_DIR_CHANGE);
                dirMsg.writeInt16(oid);
                dirMsg.writeInt8(
                        o->getComponent<BeingComponent>()->getDirection());
                gameHandler->sendTo(p, dirMsg);
            }

            // Send ability uses
            if (oflags & UPDATEFLAG_ABILITY_ON_POINT)
            {
                MessageOut abilityMsg(GPMSG_BEING_ABILITY_POINT);
                abilityMsg.writeInt16(oid);
                auto *abilityComponent = o->getComponent<AbilityComponent>();
                const Point &point = abilityComponent->getLastTargetPoint();
                abilityMsg.writeInt8(abilityComponent->getLastUsedAbilityId());
                abilityMsg.writeInt16(point.x);
                abilityMsg.writeInt16(point.y);
                gameHandler->sendTo(p, abilityMsg);
            }

            if (oflags & UPDATEFLAG_ABILITY_ON_BEING)
            {
                MessageOut abilityMsg(GPMSG_BEING_ABILITY_BEING);
                abilityMsg.writeInt16(oid);
                auto *abilityComponent = o->getComponent<AbilityComponent>();
                abilityMsg.writeInt8(abilityComponent->getLastUsedAbilityId());
                abilityMsg.writeInt16(
                        abilityComponent->getLastTargetBeingId());
                gameHandler->sendTo(p, abilityMsg);
            }

            if (oflags & UPDATEFLAG_ABILITY_ON_DIRECTION)
            {
                MessageOut abilityMsg(GPMSG_BEING_ABILITY_DIRECTION);
                abilityMsg.writeInt16(oid);
                auto *abilityComponent = o->getComponent<AbilityComponent>();
                abilityMsg.writeInt8(abilityComponent->getLastUsedAbilityId());
                abilityMsg.writeInt8(
                        abilityComponent->getLastTargetDirection());
                gameHandler->sendTo(p, abilityMsg);
            }

            // Send damage messages.
            if (o->canFight())
            {
                auto *beingComponent = o->getComponent<BeingComponent>();
                const Hits &hits = beingComponent->getHitsTaken();
                for (Hits::const_iterator j = hits.begin(),
                     j_end = hits.end(); j != j_end; ++j)
                {
                    damageMsg.writeInt16(oid);
                    damageMsg.writeInt16(*j);
                }
            }

            if (oold == opos)
            {
                // o does not move, nothing more to report.
                continue;
            }
        }

        if (!willBeInRange)
        {
            // o is no longer visible from p. Send leave message.
            MessageOut leaveMsg(GPMSG_BEING_LEAVE);
            leaveMsg.writeInt16(oid);
            gameHandler->sendTo(p, leaveMsg);
            continue;
        }

        if (!wereInRange)
        {
            // o is now visible by p. Send enter message.
            MessageOut enterMsg(GPMSG_BEING_ENTER);
            enterMsg.writeInt8(otype);
            enterMsg.writeInt16(oid);
            enterMsg.writeInt8(o->getComponent<BeingComponent>()->getAction());
            enterMsg.writeInt16(opos.x);
            enterMsg.writeInt16(opos.y);
            enterMsg.writeInt8(
                    o->getComponent<BeingComponent>()->getDirection());
            enterMsg.writeInt8(o->getComponent<BeingComponent>()->getGender());
            switch (otype)
            {
                case OBJECT_CHARACTER:
                {
                    enterMsg.writeString(
                            o->getComponent<BeingComponent>()->getName());
                    serializeLooks(o, enterMsg);
                } break;

                case OBJECT_MONSTER:
                {
                    MonsterComponent *monsterComponent =
                            o->getComponent<MonsterComponent>();
                    enterMsg.writeInt16(monsterComponent->getSpecy()->getId());
                    enterMsg.writeString(
                            o->getComponent<BeingComponent>()->getName());
                } break;

                case OBJECT_NPC:
                {
                    NpcComponent *npcComponent =
                            o->getComponent<NpcComponent>();
                    enterMsg.writeInt16(npcComponent->getNpcId());
                    enterMsg.writeString(
                            o->getComponent<BeingComponent>()->getName());
                } break;

                default:
                    assert(false); // TODO
                    break;
            }
            gameHandler->sendTo(p, enterMsg);
        }

        if (opos != oold)
        {
            // Add position check coords every 5 seconds.
            if (currentTick % 50 == 0)
                flags |= MOVING_POSITION;

            flags |= MOVING_DESTINATION;
        }

        // Send move messages.
        moveMsg.writeInt16(oid);
        moveMsg.writeInt8(flags);
        if (flags & MOVING_POSITION)
        {
            moveMsg.writeInt16(oold.x);
            moveMsg.writeInt16(oold.y);
        }

        if (flags & MOVING_DESTINATION)
        {
            moveMsg.writeInt16(opos.x);
            moveMsg.writeInt16(opos.y);
            // We multiply the sent speed (in tiles per second) by ten
            // to get it within a byte with decimal precision.
            // For instance, a value of 4.5 will be sent as 45.
            auto *tpsSpeedAttribute = attributeManager->getAttributeInfo(ATTR_MOVE_SPEED_TPS);
            moveMsg.writeInt8((unsigned short)
                (o->getComponent<BeingComponent>()
                        ->getModifiedAttribute(tpsSpeedAttribute) * 10));
        }
    }

    // Do not send a packet if nothing happened in p's range.
    if (moveMsg.getLength() > 2)
        gameHandler->sendTo(p, moveMsg);

    if (damageMsg.getLength() > 2)
        gameHandler->sendTo(p, damageMsg);

    // Inform client about status change.
    p->getComponent<CharacterComponent>()->sendStatus(*p);

    // Inform client about health change of party members
    for (CharacterIterator i(map->getWholeMapIterator()); i; ++i)
    {
        Entity *c = *i;

        // Make sure its not the same character
        if (c == p)
            continue;

        // make sure they are in the same party
        if (c->getComponent<CharacterComponent>()->getParty() ==
                p->getComponent<CharacterComponent>()->getParty())
        {
            int cflags = c->getComponent<ActorComponent>()->getUpdateFlags();
            if (cflags & UPDATEFLAG_HEALTHCHANGE)
            {
                auto *beingComponent = c->getComponent<BeingComponent>();

                MessageOut healthMsg(GPMSG_BEING_HEALTH_CHANGE);
                healthMsg.writeInt16(
                        c->getComponent<ActorComponent>()->getPublicID());
                auto *hpAttribute = attributeManager->getAttributeInfo(ATTR_HP);
                healthMsg.writeInt16(
                        beingComponent->getModifiedAttribute(hpAttribute));
                auto *maxHpAttribute = attributeManager->getAttributeInfo(ATTR_MAX_HP);
                healthMsg.writeInt16(
                        beingComponent->getModifiedAttribute(maxHpAttribute));
                gameHandler->sendTo(p, healthMsg);
            }
        }
    }

    // Inform client about items on the ground around its character
    MessageOut itemMsg(GPMSG_ITEMS);
    for (FixedActorIterator it(map->getAroundBeingIterator(p, visualRange));
         it; ++it)
    {
        Entity *o = *it;

        assert(o->getType() == OBJECT_ITEM ||
               o->getType() == OBJECT_EFFECT);

        Point opos = o->getComponent<ActorComponent>()->getPosition();
        int oflags = o->getComponent<ActorComponent>()->getUpdateFlags();
        bool willBeInRange = ppos.inRangeOf(opos, visualRange);
        bool wereInRange = pold.inRangeOf(opos, visualRange) &&
                           !((pflags | oflags) & UPDATEFLAG_NEW_ON_MAP);

        if (willBeInRange ^ wereInRange)
        {
            switch (o->getType())
            {
                case OBJECT_ITEM:
                {
                    ItemComponent *item = o->getComponent<ItemComponent>();
                    ItemClass *itemClass = item->getItemClass();

                    if (oflags & UPDATEFLAG_NEW_ON_MAP)
                    {
                        /* Send a specific message to the client when an item appears
                           out of nowhere, so that a sound/animation can be performed. */
                        MessageOut appearMsg(GPMSG_ITEM_APPEAR);
                        appearMsg.writeInt16(itemClass->getDatabaseID());
                        appearMsg.writeInt16(opos.x);
                        appearMsg.writeInt16(opos.y);
                        gameHandler->sendTo(p, appearMsg);
                    }
                    else
                    {
                        itemMsg.writeInt16(willBeInRange ? itemClass->getDatabaseID() : 0);
                        itemMsg.writeInt16(opos.x);
                        itemMsg.writeInt16(opos.y);
                    }
                }
                break;
                case OBJECT_EFFECT:
                {
                    EffectComponent *e = o->getComponent<EffectComponent>();
                    // Don't show old effects
                    if (!(oflags & UPDATEFLAG_NEW_ON_MAP))
                        break;

                    if (Entity *b = e->getBeing())
                    {
                        auto *actorComponent =
                                b->getComponent<ActorComponent>();
                        MessageOut effectMsg(GPMSG_CREATE_EFFECT_BEING);
                        effectMsg.writeInt16(e->getEffectId());
                        effectMsg.writeInt16(actorComponent->getPublicID());
                        gameHandler->sendTo(p, effectMsg);
                    } else {
                        MessageOut effectMsg(GPMSG_CREATE_EFFECT_POS);
                        effectMsg.writeInt16(e->getEffectId());
                        effectMsg.writeInt16(opos.x);
                        effectMsg.writeInt16(opos.y);
                        gameHandler->sendTo(p, effectMsg);
                    }
                }
                break;
                default: break;
            } // Switch
        }
    }

    // Do not send a packet if nothing happened in p's range.
    if (itemMsg.getLength() > 2)
        gameHandler->sendTo(p, itemMsg);
}

#ifndef NDEBUG
static bool dbgLockObjects;
#endif

void GameState::update(int tick)
{
    currentTick = tick;

#ifndef NDEBUG
    dbgLockObjects = true;
#endif

    ScriptManager::currentState()->update();

    // Update game state (update AI, etc.)
    const MapManager::Maps &maps = MapManager::getMaps();
    for (MapManager::Maps::const_iterator m = maps.begin(),
         m_end = maps.end(); m != m_end; ++m)
    {
        MapComposite *map = m->second;
        if (!map->isActive())
            continue;

        map->update();

        for (CharacterIterator p(map->getWholeMapIterator()); p; ++p)
        {
            informPlayer(map, *p);
        }

        for (ActorIterator it(map->getWholeMapIterator()); it; ++it)
        {
            Entity *a = *it;
            a->getComponent<ActorComponent>()->clearUpdateFlags();
            if (a->canFight())
            {
                a->getComponent<BeingComponent>()->clearHitsTaken();
            }
        }
    }

#   ifndef NDEBUG
    dbgLockObjects = false;
#   endif

    // Take care of events that were delayed because of their side effects.
    for (DelayedEvents::iterator it = delayedEvents.begin(),
         it_end = delayedEvents.end(); it != it_end; ++it)
    {
        const DelayedEvent &e = it->second;
        Entity *o = it->first;
        switch (e.type)
        {
            case EVENT_REMOVE:
                remove(o);
                if (o->getType() == OBJECT_CHARACTER)
                {
                    o->getComponent<CharacterComponent>()->disconnected(*o);
                    gameHandler->detachClient(o);
                }
                delete o;
                break;

            case EVENT_INSERT:
                insertOrDelete(o);
                break;

            case EVENT_WARP:
                assert(o->getType() == OBJECT_CHARACTER);
                warp(o, e.map, e.point);
                break;
        }
    }
    delayedEvents.clear();
}

bool GameState::insert(Entity *ptr)
{
    assert(!dbgLockObjects);
    MapComposite *map = ptr->getMap();
    assert(map && map->isActive());

    /* Non-visible objects have neither position nor public ID, so their
       insertion cannot fail. Take care of them first. */
    if (!ptr->isVisible())
    {
        map->insert(ptr);
        ptr->signal_inserted.emit(ptr);
        return true;
    }

    // Check that coordinates are actually valid.
    Entity *obj = static_cast< Entity * >(ptr);
    Map *mp = map->getMap();
    Point pos = obj->getComponent<ActorComponent>()->getPosition();
    if ((int)pos.x / mp->getTileWidth() >= mp->getWidth() ||
        (int)pos.y / mp->getTileHeight() >= mp->getHeight())
    {
        LOG_ERROR("Tried to insert an actor at position " << pos.x << ','
                  << pos.y << " outside map " << map->getID() << '.');
        // Set an arbitrary small position.
        pos = Point(100, 100);
        obj->getComponent<ActorComponent>()->setPosition(*ptr, pos);
    }

    if (!map->insert(obj))
    {
        // The map is overloaded, no room to add a new actor
        LOG_ERROR("Too many actors on map " << map->getID() << '.');
        return false;
    }

    obj->signal_inserted.emit(obj);

    // DEBUG INFO
    switch (obj->getType())
    {
        case OBJECT_ITEM:
            LOG_DEBUG("Item inserted: "
                   << obj->getComponent<ItemComponent>()->getItemClass()->getDatabaseID());
            break;

        case OBJECT_NPC:
            LOG_DEBUG("NPC inserted: " << obj->getComponent<NpcComponent>()->getNpcId());
            break;

        case OBJECT_CHARACTER:
            LOG_DEBUG("Player inserted: "
                      << obj->getComponent<BeingComponent>()->getName());
            break;

        case OBJECT_EFFECT:
            LOG_DEBUG("Effect inserted: "
                      << obj->getComponent<EffectComponent>()->getEffectId());
            break;

        case OBJECT_MONSTER:
        {
            MonsterComponent *monsterComponent =
                    obj->getComponent<MonsterComponent>();
            LOG_DEBUG("Monster inserted: "
                      << monsterComponent->getSpecy()->getId());
            break;
        }
        case OBJECT_OTHER:
        default:
            LOG_DEBUG("Entity inserted: " << obj->getType());
            break;
    }

    obj->getComponent<ActorComponent>()->raiseUpdateFlags(
            UPDATEFLAG_NEW_ON_MAP);
    if (obj->getType() != OBJECT_CHARACTER)
        return true;

    /* Since the player does not know yet where in the world its character is,
       we send a map-change message, even if it is the first time it
       connects to this server. */
    MessageOut mapChangeMessage(GPMSG_PLAYER_MAP_CHANGE);
    mapChangeMessage.writeString(map->getName());
    mapChangeMessage.writeInt16(pos.x);
    mapChangeMessage.writeInt16(pos.y);
    gameHandler->sendTo(ptr, mapChangeMessage);

    // update the online state of the character
    accountHandler->updateOnlineStatus(ptr->getComponent<CharacterComponent>()
                                       ->getDatabaseID(), true);

    return true;
}

int GameState::getCurrentTick()
{
    return currentTick;
}

bool GameState::insertOrDelete(Entity *ptr)
{
    if (insert(ptr)) return true;
    delete ptr;
    return false;
}

void GameState::remove(Entity *ptr)
{
    assert(!dbgLockObjects);
    MapComposite *map = ptr->getMap();
    int visualRange = Configuration::getValue("game_visualRange", 448);

    ptr->signal_removed.emit(ptr);

    // DEBUG INFO
    switch (ptr->getType())
    {
        case OBJECT_ITEM:
            LOG_DEBUG("Item removed: "
                   << ptr->getComponent<ItemComponent>()->getItemClass()->getDatabaseID());
            break;

        case OBJECT_NPC:
            LOG_DEBUG("NPC removed: " << ptr->getComponent<NpcComponent>()->getNpcId());
            break;

        case OBJECT_CHARACTER:
            LOG_DEBUG("Player removed: "
                      << ptr->getComponent<BeingComponent>()->getName());
            break;

        case OBJECT_EFFECT:
            LOG_DEBUG("Effect removed: "
                      << ptr->getComponent<EffectComponent>()->getEffectId());
            break;

        case OBJECT_MONSTER:
        {
            MonsterComponent *monsterComponent =
                    ptr->getComponent<MonsterComponent>();
            LOG_DEBUG("Monster removed: "
                      << monsterComponent->getSpecy()->getId());
            break;
        }
        case OBJECT_OTHER:
        default:
            LOG_DEBUG("Entity removed: " << ptr->getType());
            break;
    }

    if (ptr->canMove())
    {
        if (ptr->getType() == OBJECT_CHARACTER)
        {
            auto *characterComponent =
                    ptr->getComponent<CharacterComponent>();
            characterComponent->cancelTransaction();

            // remove characters online status
            accountHandler->updateOnlineStatus(
                    characterComponent->getDatabaseID(), false);
        }

        MessageOut msg(GPMSG_BEING_LEAVE);
        msg.writeInt16(ptr->getComponent<ActorComponent>()->getPublicID());
        Point objectPos = ptr->getComponent<ActorComponent>()->getPosition();

        for (CharacterIterator p(map->getAroundActorIterator(ptr, visualRange));
             p; ++p)
        {
            if (*p != ptr && objectPos.inRangeOf(
                    (*p)->getComponent<ActorComponent>()->getPosition(),
                visualRange))
            {
                gameHandler->sendTo(*p, msg);
            }
        }
    }
    else if (ptr->getType() == OBJECT_ITEM)
    {
        Point pos = ptr->getComponent<ActorComponent>()->getPosition();
        MessageOut msg(GPMSG_ITEMS);
        msg.writeInt16(0);
        msg.writeInt16(pos.x);
        msg.writeInt16(pos.y);

        for (CharacterIterator p(map->getAroundActorIterator(ptr, visualRange)); p; ++p)
        {
            const Point &point =
                    (*p)->getComponent<ActorComponent>()->getPosition();
            if (pos.inRangeOf(point, visualRange))
            {
                gameHandler->sendTo(*p, msg);
            }
        }
    }

    map->remove(ptr);
}

void GameState::warp(Entity *ptr, MapComposite *map, const Point &point)
{
    remove(ptr);

    ptr->setMap(map);
    ptr->getComponent<ActorComponent>()->setPosition(*ptr, point);
    ptr->getComponent<BeingComponent>()->clearDestination(*ptr);
    /* Force update of persistent data on map change, so that
       characters can respawn at the start of the map after a death or
       a disconnection. */
    accountHandler->sendCharacterData(ptr);

    auto *characterComponent =
            ptr->getComponent<CharacterComponent>();

    // If the player has just left, The character pointer is also about
    // to be deleted. So we don't have to do anything else.
    if (!characterComponent->isConnected())
        return;

    if (map->isActive())
    {
        if (!insert(ptr))
        {
            characterComponent->disconnected(*ptr);
            gameHandler->detachClient(ptr);
            delete ptr;
        }
    }
    else
    {
        MessageOut msg(GAMSG_REDIRECT);
        msg.writeInt32(characterComponent->getDatabaseID());
        accountHandler->send(msg);
        gameHandler->prepareServerChange(ptr);
    }
}

/**
 * Enqueues an event. It will be executed at end of update.
 */
static void enqueueEvent(Entity *ptr, const DelayedEvent &e)
{
    std::pair< DelayedEvents::iterator, bool > p =
        delayedEvents.insert(std::make_pair(ptr, e));
    // Delete events take precedence over other events.
    if (!p.second && e.type == EVENT_REMOVE)
    {
        p.first->second.type = EVENT_REMOVE;
    }
}

void GameState::enqueueInsert(Entity *ptr)
{
    DelayedEvent event;
    event.type = EVENT_INSERT;
    event.map = 0;
    enqueueEvent(ptr, event);
}

void GameState::enqueueRemove(Entity *ptr)
{
    DelayedEvent event;
    event.type = EVENT_REMOVE;
    event.map = 0;
    enqueueEvent(ptr, event);
}

void GameState::enqueueWarp(Entity *ptr,
                            MapComposite *map,
                            const Point &point)
{
    // When the player has just disconnected, better not wait for the pointer
    // to become invalid.
    if (!ptr->getComponent<CharacterComponent>()->isConnected())
    {
        warp(ptr, map, point);
        return;
    }

    DelayedEvent event;
    event.type = EVENT_WARP;
    event.point = point;
    event.map = map;
    enqueueEvent(ptr, event);
}

void GameState::sayAround(Entity *entity, const std::string &text)
{
    Point speakerPosition = entity->getComponent<ActorComponent>()->getPosition();
    int visualRange = Configuration::getValue("game_visualRange", 448);

    for (CharacterIterator i(entity->getMap()->getAroundActorIterator(entity, visualRange)); i; ++i)
    {
        const Point &point =
                (*i)->getComponent<ActorComponent>()->getPosition();
        if (speakerPosition.inRangeOf(point, visualRange))
        {
            sayTo(*i, entity, text);
        }
    }
}

void GameState::sayTo(Entity *destination, Entity *source, const std::string &text)
{
    if (destination->getType() != OBJECT_CHARACTER)
        return; //only characters will read it anyway

    MessageOut msg(GPMSG_SAY);
    if (source == nullptr)
    {
        msg.writeInt16(0);
    }
    else if (!source->canMove())
    {
        msg.writeInt16(65535);
    }
    else
    {
        msg.writeInt16(source->getComponent<ActorComponent>()->getPublicID());
    }
    msg.writeString(text);

    gameHandler->sendTo(destination, msg);
}

void GameState::sayToAll(const std::string &text)
{
    MessageOut msg(GPMSG_SAY);

    // The message will be shown as if it was from the server
    msg.writeInt16(0);
    msg.writeString(text);

    // Sends it to everyone connected to the game server
    gameHandler->sendToEveryone(msg);
}


std::string GameState::getVariable(const std::string &key)
{
    std::map<std::string, std::string>::iterator iValue =
                                                     mScriptVariables.find(key);
    if (iValue != mScriptVariables.end())
        return iValue->second;
    else
        return std::string();
}

void GameState::setVariable(const std::string &key, const std::string &value)
{
    if (mScriptVariables[key] == value)
        return;
    mScriptVariables[key] = value;
    accountHandler->updateWorldVar(key, value);
    callVariableCallbacks(key, value);
}

void GameState::setVariableFromDbserver(const std::string &key,
                                        const std::string &value)
{
    if (mScriptVariables[key] == value)
        return;
    mScriptVariables[key] = value;
    callVariableCallbacks(key, value);
}

void GameState::callVariableCallbacks(const std::string &key,
                                      const std::string &value)
{
    const MapManager::Maps &maps = MapManager::getMaps();
    for (MapManager::Maps::const_iterator m = maps.begin(),
         m_end = maps.end(); m != m_end; ++m)
    {
        m->second->callWorldVariableCallback(key, value);
    }
}
