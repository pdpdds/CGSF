/*
 *  The Mana Server
 *  Copyright (C) 2013 The Mana Developers
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

#include "abilitycomponent.h"

#include "being.h"
#include "entity.h"

#include "scripting/scriptmanager.h"

#include "utils/logger.h"

AbilityComponent::AbilityComponent():
    mLastUsedAbilityId(0),
    mLastTargetBeingId(0)
{
}

void AbilityComponent::update(Entity &entity)
{
    // Update ability recharge
    for (auto &it : mAbilities)
    {
        auto &ability = it.second;
        if (!ability.recharged && ability.rechargeTimeout.expired()) {
            ability.recharged = true;

            if (ability.abilityInfo->rechargedCallback.isValid()) {
                Script *script = ScriptManager::currentState();
                script->prepare(ability.abilityInfo->rechargedCallback);
                script->push(&entity);
                script->push(ability.abilityInfo->id);
                script->execute(entity.getMap());
            }
        }
    }

}

/**
 * Removes an available ability action
 */
bool AbilityComponent::takeAbility(int id)
{
    AbilityMap::iterator i = mAbilities.find(id);
    if (i != mAbilities.end())
    {
        mAbilities.erase(i);
        signal_ability_took.emit(id);
        return true;
    }
    return false;
}

bool AbilityComponent::abilityUseCheck(AbilityMap::iterator it)
{
    if (!mGlobalCooldown.expired())
        return false;

    if (it == mAbilities.end())
    {
        LOG_INFO("Entity uses ability without authorization.");
        return false;
    }

    //check if the ability is currently recharged
    AbilityValue &ability = it->second;
    if (!ability.recharged)
    {
        LOG_INFO("Character uses ability " << it->first
                 << " which is not recharged. ("
                 << ability.rechargeTimeout.remaining()
                 << " ticks are missing)");
        return false;
    }

    if (!ability.abilityInfo->useCallback.isValid())
    {
        LOG_WARN("No callback for use of ability "
                 << ability.abilityInfo->name << ". Ignoring ability.");
        return false;
    }
    return true;
}

/**
 * makes the character perform a ability on a being
 * when it is allowed to do so
 */
bool AbilityComponent::useAbilityOnBeing(Entity &user, int id, Entity *b)
{
    AbilityMap::iterator it = mAbilities.find(id);
    if (!abilityUseCheck(it))
            return false;

    AbilityValue &ability = it->second;

    if (ability.abilityInfo->target != AbilityManager::TARGET_BEING)
        return false;

    //tell script engine to cast the spell
    Script *script = ScriptManager::currentState();
    script->prepare(ability.abilityInfo->useCallback);
    script->push(&user);
    script->push(b);
    script->push(ability.abilityInfo->id);
    script->execute(user.getMap());

    mLastUsedAbilityId = id;
    if (b)
        mLastTargetBeingId = b->getComponent<ActorComponent>()->getPublicID();
    else
        mLastTargetBeingId = 0;
    user.getComponent<ActorComponent>()->raiseUpdateFlags(
            UPDATEFLAG_ABILITY_ON_BEING);
    return true;
}

/**
 * makes the character perform a ability on a map point
 * when it is allowed to do so
 */
bool AbilityComponent::useAbilityOnPoint(Entity &user, int id, int x, int y)
{
    AbilityMap::iterator it = mAbilities.find(id);
    if (!abilityUseCheck(it))
            return false;

    AbilityValue &ability = it->second;

    if (ability.abilityInfo->target != AbilityManager::TARGET_POINT)
        return false;

    //tell script engine to cast the spell
    Script *script = ScriptManager::currentState();
    script->prepare(ability.abilityInfo->useCallback);
    script->push(&user);
    script->push(x);
    script->push(y);
    script->push(ability.abilityInfo->id);
    script->execute(user.getMap());

    mLastUsedAbilityId = id;
    mLastTargetPoint = Point(x, y);
    user.getComponent<ActorComponent>()->raiseUpdateFlags(
            UPDATEFLAG_ABILITY_ON_POINT);
    return true;
}

bool AbilityComponent::useAbilityOnDirection(Entity &user, int id,
                                             ManaServ::BeingDirection direction)
{
    AbilityMap::iterator it = mAbilities.find(id);
    if (!abilityUseCheck(it))
            return false;

    AbilityValue &ability = it->second;

    if (ability.abilityInfo->target != AbilityManager::TARGET_DIRECTION)
        return false;

    //tell script engine to cast the spell
    Script *script = ScriptManager::currentState();
    script->prepare(ability.abilityInfo->useCallback);
    script->push(&user);
    script->push(direction);
    script->push(ability.abilityInfo->id);
    script->execute(user.getMap());

    mLastUsedAbilityId = id;
    mLastTargetDirection = direction;
    user.getComponent<ActorComponent>()->raiseUpdateFlags(
            UPDATEFLAG_ABILITY_ON_DIRECTION);
    return true;
}

/**
 * Allows a character to perform a ability
 */
bool AbilityComponent::giveAbility(int id)
{
    if (mAbilities.find(id) == mAbilities.end())
    {
        auto *abilityInfo = abilityManager->getAbilityInfo(id);
        if (!abilityInfo)
        {
            LOG_ERROR("Tried to give not existing ability id " << id << ".");
            return false;
        }
        return giveAbility(abilityInfo);
    }
    return false;
}

bool AbilityComponent::giveAbility(const AbilityManager::AbilityInfo *info)
{
    bool added = mAbilities.insert(std::pair<int, AbilityValue>(info->id,
                                   AbilityValue(info))).second;

    signal_ability_changed.emit(info->id);
    return added;
}

/**
 * Sets cooldown time for this ability
 */
void AbilityComponent::setAbilityCooldown(int id, int ticks)
{
    AbilityMap::iterator it = mAbilities.find(id);
    if (it != mAbilities.end())
    {
        it->second.recharged = false;
        it->second.rechargeTimeout.set(ticks);
        signal_ability_changed.emit(id);
    }
}

int AbilityComponent::abilityCooldown(int id)
{
    AbilityMap::iterator it = mAbilities.find(id);
    if (it != mAbilities.end() && !it->second.recharged)
        return it->second.rechargeTimeout.remaining();

    return 0;
}

void AbilityComponent::setGlobalCooldown(int ticks)
{
    // Enforce a minimum cooldown of 1 tick to prevent syncing issues
    ticks = std::max(ticks, 1);
    mGlobalCooldown.set(ticks);
    signal_global_cooldown_activated.emit();
}

