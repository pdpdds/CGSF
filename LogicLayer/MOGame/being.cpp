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

#include "being.h"

#include "common/configuration.h"
#include "common/defines.h"
#include "attributemanager.h"
#include "charactercomponent.h"
#include "collisiondetection.h"
#include "mapcomposite.h"
#include "effect.h"
#include "statuseffect.h"
#include "statusmanager.h"
#include "utils/logger.h"
#include "utils/speedconv.h"
#include "scripting/scriptmanager.h"


Script::Ref BeingComponent::mRecalculateDerivedAttributesCallback;
Script::Ref BeingComponent::mRecalculateBaseAttributeCallback;

BeingComponent::BeingComponent(Entity &entity):
    mMoveTime(0),
    mAction(STAND),
    mGender(GENDER_UNSPECIFIED),
    mDirection(DOWN),
    mEmoteId(0)
{
    auto &attributeScope = attributeManager->getAttributeScope(BeingScope);
    LOG_DEBUG("Being creation: initialisation of " << attributeScope.size()
              << " attributes.");
    for (auto &attribute : attributeScope)
    {
        LOG_DEBUG("Attempting to create attribute '"
                  << attribute->id << "'.");
        mAttributes.insert(std::make_pair(attribute, Attribute(attribute)));
    }

    clearDestination(entity);

    entity.signal_inserted.connect(sigc::mem_fun(this,
                                                 &BeingComponent::inserted));

    // TODO: Way to define default base values?
    // Should this be handled by the virtual modifiedAttribute?
    // URGENT either way
#if 0
    // Initialize element resistance to 100 (normal damage).
    for (i = BASE_ELEM_BEGIN; i < BASE_ELEM_END; ++i)
    {
        mAttributes[i] = Attribute(TY_ST);
        mAttributes[i].setBase(100);
    }
#endif
}

void BeingComponent::triggerEmote(Entity &entity, int id)
{
    mEmoteId = id;

    if (id > -1)
        entity.getComponent<ActorComponent>()->raiseUpdateFlags(UPDATEFLAG_EMOTE);
}



void BeingComponent::heal(Entity &entity)
{
    auto *hpAttribute = attributeManager->getAttributeInfo(ATTR_HP);
    Attribute &hp = mAttributes.at(hpAttribute);
    Attribute &maxHp = mAttributes.at(attributeManager->getAttributeInfo(ATTR_MAX_HP));
    if (maxHp.getModifiedAttribute() == hp.getModifiedAttribute())
        return; // Full hp, do nothing.

    // Reset all modifications present in hp.
    hp.clearMods();
    setAttribute(entity, hpAttribute, maxHp.getModifiedAttribute());
}

void BeingComponent::heal(Entity &entity, int gain)
{
    auto *hpAttribute = attributeManager->getAttributeInfo(ATTR_HP);
    auto *maxHpAttribute = attributeManager->getAttributeInfo(ATTR_MAX_HP);
    Attribute &hp = mAttributes.at(hpAttribute);
    Attribute &maxHp = mAttributes.at(maxHpAttribute);
    if (maxHp.getModifiedAttribute() == hp.getModifiedAttribute())
        return; // Full hp, do nothing.

    // Cannot go over maximum hitpoints.
    setAttribute(entity, hpAttribute, hp.getBase() + gain);
    if (hp.getModifiedAttribute() > maxHp.getModifiedAttribute())
        setAttribute(entity, hpAttribute, maxHp.getModifiedAttribute());
}

void BeingComponent::died(Entity &entity)
{
    if (mAction == DEAD)
        return;

    LOG_DEBUG("Being " << entity.getComponent<ActorComponent>()->getPublicID()
              << " died.");
    setAction(entity, DEAD);
    // dead beings stay where they are
    clearDestination(entity);

    signal_died.emit(&entity);
}

void BeingComponent::setDestination(Entity &entity, const Point &dst)
{
    mDst = dst;
    entity.getComponent<ActorComponent>()->raiseUpdateFlags(
            UPDATEFLAG_NEW_DESTINATION);
    mPath.clear();
}

void BeingComponent::clearDestination(Entity &entity)
{
    setDestination(entity,
                   entity.getComponent<ActorComponent>()->getPosition());
}

void BeingComponent::setDirection(Entity &entity, BeingDirection direction)
{
    mDirection = direction;
    entity.getComponent<ActorComponent>()->raiseUpdateFlags(
            UPDATEFLAG_DIRCHANGE);
}

Path BeingComponent::findPath(Entity &entity)
{
    auto *actorComponent = entity.getComponent<ActorComponent>();

    Map *map = entity.getMap()->getMap();
    int tileWidth = map->getTileWidth();
    int tileHeight = map->getTileHeight();
    int startX = actorComponent->getPosition().x / tileWidth;
    int startY = actorComponent->getPosition().y / tileHeight;
    int destX = mDst.x / tileWidth, destY = mDst.y / tileHeight;

    return map->findPath(startX, startY, destX, destY,
                         actorComponent->getWalkMask());
}

void BeingComponent::updateDirection(Entity &entity,
                                     const Point &currentPos,
                                     const Point &destPos)
{
    // We update the being direction on each tile to permit other beings
    // entering in range to always see the being with a direction value.

    if (currentPos == destPos)
        return;

    const int dx = destPos.x - currentPos.x;
    const int dy = destPos.y - currentPos.y;

    if (std::abs(dx) > std::abs(dy))
        setDirection(entity, (dx < 0) ? LEFT : RIGHT);
    else
        setDirection(entity, (dy < 0) ? UP : DOWN);
}

void BeingComponent::move(Entity &entity)
{
    // Immobile beings cannot move.
    if (!checkAttributeExists(attributeManager->getAttributeInfo(ATTR_MOVE_SPEED_RAW))
        || !getModifiedAttribute(attributeManager->getAttributeInfo(ATTR_MOVE_SPEED_RAW)))
          return;

    // Remember the current position before moving. This is used by
    // MapComposite::update() to determine whether a being has moved from one
    // zone to another.
    mOld = entity.getComponent<ActorComponent>()->getPosition();

    // Ignore not moving or dead beings
    if ((mAction == STAND && mDst == mOld) || mAction == DEAD)
        return;

    if (mMoveTime > WORLD_TICK_MS)
    {
        // Current move has not yet ended
        mMoveTime -= WORLD_TICK_MS;
        return;
    }

    Map *map = entity.getMap()->getMap();
    int tileWidth = map->getTileWidth();
    int tileHeight = map->getTileHeight();
    int tileSX = mOld.x / tileWidth;
    int tileSY = mOld.y / tileHeight;
    int tileDX = mDst.x / tileWidth;
    int tileDY = mDst.y / tileHeight;

    if (tileSX == tileDX && tileSY == tileDY)
    {
        if (mAction == WALK)
            setAction(entity, STAND);
        // Moving while staying on the same tile is free
        // We only update the direction in that case.
        updateDirection(entity, mOld, mDst);
        entity.getComponent<ActorComponent>()->setPosition(entity, mDst);
        mMoveTime = 0;
        return;
    }

    /* If no path exists, the for-loop won't be entered. Else a path for the
     * current destination has already been calculated.
     * The tiles in this path have to be checked for walkability,
     * in case there have been changes. The 'getWalk' method of the Map
     * class has been used, because that seems to be the most logical
     * place extra functionality will be added.
     */
    for (Point &point : mPath)
    {
        const unsigned char walkmask =
                entity.getComponent<ActorComponent>()->getWalkMask();
        if (!map->getWalk(point.x, point.y, walkmask))
        {
            mPath.clear();
            break;
        }
    }

    if (mPath.empty())
    {
        // No path exists: the walkability of cached path has changed, the
        // destination has changed, or a path was never set.
        mPath = findPath(entity);
    }

    if (mPath.empty())
    {
        if (mAction == WALK)
            setAction(entity, STAND);
        // no path was found
        mDst = mOld;
        mMoveTime = 0;
        return;
    }

    setAction(entity, WALK);

    Point prev(tileSX, tileSY);
    Point pos;
    do
    {
        Point next = mPath.front();
        mPath.pop_front();

        auto *rawSpeedAttribute = attributeManager->getAttributeInfo(ATTR_MOVE_SPEED_RAW);
        // SQRT2 is used for diagonal movement.
        mMoveTime += (prev.x == next.x || prev.y == next.y) ?
                       getModifiedAttribute(rawSpeedAttribute) :
                       getModifiedAttribute(rawSpeedAttribute) * SQRT2;

        if (mPath.empty())
        {
            // skip last tile center
            pos = mDst;
            break;
        }

        // Position the actor in the middle of the tile for pathfinding purposes
        pos.x = next.x * tileWidth + (tileWidth / 2);
        pos.y = next.y * tileHeight + (tileHeight / 2);
    }
    while (mMoveTime < WORLD_TICK_MS);
    entity.getComponent<ActorComponent>()->setPosition(entity, pos);

    mMoveTime = mMoveTime > WORLD_TICK_MS ? mMoveTime - WORLD_TICK_MS : 0;

    // Update the being direction also
    updateDirection(entity, mOld, pos);
}

int BeingComponent::directionToAngle(int direction)
{
    switch (direction)
    {
        case UP:    return  90;
        case DOWN:  return 270;
        case RIGHT: return 180;
        case LEFT:
        default:    return   0;
    }
}

void BeingComponent::setAction(Entity &entity, BeingAction action)
{
    mAction = action;
    // The players are informed about these actions by other messages
    if (action != WALK)
    {
        entity.getComponent<ActorComponent>()->raiseUpdateFlags(
                UPDATEFLAG_ACTIONCHANGE);
    }
}

void BeingComponent::applyModifier(Entity &entity, AttributeInfo *attribute,
                                   double value, unsigned layer,
                                   unsigned duration, unsigned id)
{
    mAttributes.at(attribute).add(duration, value, layer, id);
    updateDerivedAttributes(entity, attribute);
}

bool BeingComponent::removeModifier(Entity &entity, AttributeInfo *attribute,
                                    double value, unsigned layer,
                                    unsigned id, bool fullcheck)
{
    bool ret = mAttributes.at(attribute).remove(value, layer, id, fullcheck);
    updateDerivedAttributes(entity, attribute);
    return ret;
}

void BeingComponent::setGender(BeingGender gender)
{
    mGender = gender;
}

void BeingComponent::setAttribute(Entity &entity,
                                  AttributeInfo *attribute,
                                  double value)
{
    auto attributeIt = mAttributes.find(attribute);
    if (attributeIt == mAttributes.end())
    {
        /*
         * The attribute does not yet exist, so we must attempt to create it.
         */
        LOG_ERROR("Being: Attempt to access non-existing attribute '"
                  << attribute->id << "'!");
        LOG_WARN("Being: Creation of new attributes dynamically is not "
                 "implemented yet!");
    }
    else
    {
        attributeIt->second.setBase(value);
        updateDerivedAttributes(entity, attribute);
    }
}

void BeingComponent::createAttribute(AttributeInfo *attributeInfo)
{
    mAttributes.insert(std::pair<AttributeInfo *, Attribute>
            (attributeInfo, Attribute(attributeInfo)));
}

const Attribute *BeingComponent::getAttribute(AttributeInfo *attribute) const
{
    AttributeMap::const_iterator ret = mAttributes.find(attribute);
    if (ret == mAttributes.end())
    {
        LOG_DEBUG("BeingComponent::getAttribute: Attribute "
                  << attribute->id << " not found! Returning 0.");
        return 0;
    }
    return &ret->second;
}

double BeingComponent::getAttributeBase(AttributeInfo *attribute) const
{
    AttributeMap::const_iterator ret = mAttributes.find(attribute);
    if (ret == mAttributes.end())
    {
        LOG_DEBUG("BeingComponent::getAttributeBase: Attribute "
                  << attribute->id << " not found! Returning 0.");
        return 0;
    }
    return ret->second.getBase();
}


double BeingComponent::getModifiedAttribute(AttributeInfo *attribute) const
{
    AttributeMap::const_iterator ret = mAttributes.find(attribute);
    if (ret == mAttributes.end())
    {
        LOG_DEBUG("BeingComponent::getModifiedAttribute: Attribute "
                  << attribute->id << " not found! Returning 0.");
        return 0;
    }
    return ret->second.getModifiedAttribute();
}

void BeingComponent::recalculateBaseAttribute(Entity &entity,
                                              AttributeInfo *attribute)
{
    LOG_DEBUG("Being: Received update attribute recalculation request for "
              << attribute << ".");
    if (!mAttributes.count(attribute))
    {
        LOG_DEBUG("BeingComponent::recalculateBaseAttribute: " << attribute->id << " not found!");
        return;
    }

    // Handle speed conversion inside the engine
    if (attribute == attributeManager->getAttributeInfo(ATTR_MOVE_SPEED_RAW))
    {
        auto *speedTpsAttribute = attributeManager->getAttributeInfo(ATTR_MOVE_SPEED_TPS);
        double newBase = utils::tpsToRawSpeed(
                                    getModifiedAttribute(speedTpsAttribute));
        if (newBase != getAttributeBase(attribute))
            setAttribute(entity, attribute, newBase);
        return;
    }

    if (!mRecalculateBaseAttributeCallback.isValid())
        return;

    Script *script = ScriptManager::currentState();
    script->prepare(mRecalculateBaseAttributeCallback);
    script->push(&entity);
    script->push(attribute);
    script->execute(entity.getMap());
}

void BeingComponent::updateDerivedAttributes(Entity &entity,
                                             AttributeInfo *attribute)
{
    signal_attribute_changed.emit(&entity, attribute);

    LOG_DEBUG("Being: Updating derived attribute(s) of: " << attribute);

    // Handle default actions before handing over to the script engine
    switch (attribute->id)
    {
    case ATTR_MAX_HP:
    case ATTR_HP:
        entity.getComponent<ActorComponent>()->raiseUpdateFlags(
                UPDATEFLAG_HEALTHCHANGE);
        break;
    case ATTR_MOVE_SPEED_TPS:
        // Does not make a lot of sense to have in the scripts.
        // So handle it here:
        recalculateBaseAttribute(entity,
                                 attributeManager->getAttributeInfo(ATTR_MOVE_SPEED_RAW));
        break;
    }

    if (!mRecalculateDerivedAttributesCallback.isValid())
        return;

    Script *script = ScriptManager::currentState();
    script->prepare(mRecalculateDerivedAttributesCallback);
    script->push(&entity);
    script->push(attribute);
    script->execute(entity.getMap());
}

void BeingComponent::applyStatusEffect(int id, int timer)
{
    if (mAction == DEAD)
        return;

    if (StatusEffect *statusEffect = StatusManager::getStatus(id))
    {
        Status newStatus;
        newStatus.status = statusEffect;
        newStatus.time = timer;
        mStatus[id] = newStatus;
    }
    else
    {
        LOG_ERROR("No status effect with ID " << id);
    }
}

void BeingComponent::removeStatusEffect(int id)
{
    setStatusEffectTime(id, 0);
}

bool BeingComponent::hasStatusEffect(int id) const
{
    for (auto &statusIt : mStatus)
    {
        if (statusIt.second.status->getId() == id)
            return true;
    }
    return false;
}

unsigned BeingComponent::getStatusEffectTime(int id) const
{
    StatusEffects::const_iterator it = mStatus.find(id);
    if (it != mStatus.end()) return it->second.time;
    else return 0;
}

void BeingComponent::setStatusEffectTime(int id, int time)
{
    StatusEffects::iterator it = mStatus.find(id);
    if (it != mStatus.end()) it->second.time = time;
}

void BeingComponent::update(Entity &entity)
{
    auto *hpAttribute = attributeManager->getAttributeInfo(ATTR_HP);

    int oldHP = getModifiedAttribute(hpAttribute);
    int newHP = oldHP;
    int maxHP = getModifiedAttribute(attributeManager->getAttributeInfo(ATTR_MAX_HP));

    // Regenerate HP
    if (mAction != DEAD && mHealthRegenerationTimeout.expired())
    {
        mHealthRegenerationTimeout.set(TICKS_PER_HP_REGENERATION);
        newHP += getModifiedAttribute(attributeManager->getAttributeInfo(ATTR_HP_REGEN));
    }
    // Cap HP at maximum
    if (newHP > maxHP)
    {
        newHP = maxHP;
    }
    // Only update HP when it actually changed to avoid network noise
    if (newHP != oldHP)
    {
        setAttribute(entity, hpAttribute, newHP);
        entity.getComponent<ActorComponent>()->raiseUpdateFlags(
                UPDATEFLAG_HEALTHCHANGE);
    }

    // Update lifetime of effects.
    for (AttributeMap::iterator it = mAttributes.begin();
         it != mAttributes.end();
         ++it)
    {
        if (it->second.tick())
            updateDerivedAttributes(entity, it->first);
    }

    // Update and run status effects
    StatusEffects::iterator it = mStatus.begin();
    while (it != mStatus.end())
    {
        it->second.time--;
        if (it->second.time > 0 && mAction != DEAD)
            it->second.status->tick(entity, it->second.time);

        if (it->second.time <= 0 || mAction == DEAD)
        {
            StatusEffects::iterator removeIt = it;
            ++it; // bring this iterator to the safety of the next element
            mStatus.erase(removeIt);
        }
        else
        {
            ++it;
        }
    }

    // Check if being died
    if (getModifiedAttribute(hpAttribute) <= 0 && mAction != DEAD)
        died(entity);
}

void BeingComponent::inserted(Entity *entity)
{
    // Reset the old position, since after insertion it is important that it is
    // in sync with the zone that we're currently present in.
    mOld = entity->getComponent<ActorComponent>()->getPosition();
}
