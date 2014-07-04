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

#ifndef ABILITYCOMPONENT_H_
#define ABILITYCOMPONENT_H_

#include "abilitymanager.h"
#include "component.h"
#include "timeout.h"

#include "utils/point.h"

#include <sigc++/signal.h>

struct AbilityValue
{
    AbilityValue(const AbilityManager::AbilityInfo *abilityInfo)
        : recharged(false)
        , abilityInfo(abilityInfo)
    {}

    bool recharged;
    Timeout rechargeTimeout;
    const AbilityManager::AbilityInfo *abilityInfo;
};

/**
 * Stores abilities by their id.
 */
typedef std::map<unsigned, AbilityValue> AbilityMap;


class AbilityComponent: public Component
{
public:
    static const ComponentType type = CT_Ability;

    AbilityComponent();

    void update(Entity &entity);

    bool useAbilityOnBeing(Entity &user, int id, Entity *b);
    bool useAbilityOnPoint(Entity &user, int id, int x, int y);
    bool useAbilityOnDirection(Entity &user, int id,
                               ManaServ::BeingDirection direction);

    bool giveAbility(int id);
    bool giveAbility(const AbilityManager::AbilityInfo *info);
    bool hasAbility(int id) const;
    bool takeAbility(int id);
    AbilityMap::iterator findAbility(int id);
    const AbilityMap &getAbilities() const;
    void clearAbilities();

    void setAbilityCooldown(int id, int ticks);
    int abilityCooldown(int id);

    void setGlobalCooldown(int ticks);
    int globalCooldown() const;

    sigc::signal<void, int> signal_ability_changed;
    sigc::signal<void, int> signal_ability_took;
    sigc::signal<void> signal_global_cooldown_activated;

    // For informing clients
    int getLastUsedAbilityId() const;

    int getLastTargetBeingId() const;
    const Point &getLastTargetPoint() const;
    ManaServ::BeingDirection getLastTargetDirection() const;

private:
    bool abilityUseCheck(AbilityMap::iterator it);

    Timeout mGlobalCooldown;

    AbilityMap mAbilities;

    // Variables required for informing clients
    int mLastUsedAbilityId;
    Point mLastTargetPoint;
    ManaServ::BeingDirection mLastTargetDirection;
    int mLastTargetBeingId;
};


/**
 * Gets the ability value by id
 */
inline AbilityMap::iterator AbilityComponent::findAbility(int id)
{
    return mAbilities.find(id);
}

/**
 * Removes all abilities from character
 */
inline void AbilityComponent::clearAbilities()
{
    mAbilities.clear();
}

/**
 * Checks if a character knows a ability action
 */
inline bool AbilityComponent::hasAbility(int id) const
{
    return mAbilities.find(id) != mAbilities.end();
}

inline const AbilityMap &AbilityComponent::getAbilities() const
{
    return mAbilities;
}

inline int AbilityComponent::globalCooldown() const
{
    return mGlobalCooldown.remaining();
}

inline int AbilityComponent::getLastUsedAbilityId() const
{
    return mLastUsedAbilityId;
}

inline const Point &AbilityComponent::getLastTargetPoint() const
{
    return mLastTargetPoint;
}

inline int AbilityComponent::getLastTargetBeingId() const
{
    return mLastTargetBeingId;
}

inline ManaServ::BeingDirection AbilityComponent::getLastTargetDirection() const
{
    return mLastTargetDirection;
}

#endif /* ABILITYCOMPONENT_H_ */
