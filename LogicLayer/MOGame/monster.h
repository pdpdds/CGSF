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

#ifndef MONSTER_H
#define MONSTER_H

#include "abilitymanager.h"
#include "being.h"

#include "common/defines.h"

#include "scripting/script.h"

#include "utils/string.h"

#include <map>
#include <set>
#include <string>
#include <vector>

#include <sigc++/connection.h>

class CharacterComponent;
class ItemClass;
class Script;

/**
 * Structure containing an item class and its probability to be dropped
 * (unit: 1/10000).
 */
struct MonsterDrop
{
    ItemClass *item;
    int probability;
};

typedef std::vector< MonsterDrop > MonsterDrops;

typedef std::map<Element, double> Vulnerabilities;

/**
 * Class describing the characteristics of a generic monster.
 */
class MonsterClass
{
    public:
        MonsterClass(int id):
            mId(id),
            mName("unnamed"),
            mGender(GENDER_UNSPECIFIED),
            mSpeed(1),
            mSize(16),
            mMutation(0),
            mOptimalLevel(0)
        {}

        /**
         * Returns monster type. This is the Id of the monster class.
         */
        int getId() const
        { return mId; }

        /**
         * Returns the name of the monster type
         */
        const std::string &getName() const
        { return mName; }

        /**
         * Sets the name of the monster type
         */
        void setName(const std::string &name)
        { mName = name; }

        void setGender(BeingGender gender)
        { mGender = gender; }

        BeingGender getGender() const
        { return mGender; }

        /**
         * Sets monster drops. These are the items the monster drops when it
         * dies.
         */
        void setDrops(const MonsterDrops &v)
        { mDrops = v; }

        /**
         * Sets a being base attribute.
         */
        void setAttribute(AttributeInfo *attribute, double value);

        const std::map<AttributeInfo *, double>
        &getAttributes() const;

        /** Sets collision circle radius. */
        void setSize(int size) { mSize = size; }

        /** Returns collision circle radius. */
        int getSize() const { return mSize; }

        /** Sets mutation factor in percent. */
        void setMutation(unsigned factor) { mMutation = factor; }

        /** Returns mutation factor in percent. */
        unsigned getMutation() const { return mMutation; }

        void addAbility(AbilityManager::AbilityInfo *info);
        const std::set<AbilityManager::AbilityInfo *> &getAbilities() const;

        void setUpdateCallback(Script *script)
        { script->assignCallback(mUpdateCallback); }

        Script::Ref getUpdateCallback() const
        { return mUpdateCallback; }

    private:
        unsigned short mId;
        std::string mName;
        BeingGender mGender;

        MonsterDrops mDrops;
        std::map<AttributeInfo *, double> mAttributes;
        std::set<AbilityManager::AbilityInfo *> mAbilities;
        float mSpeed; /**< The monster class speed in tiles per second */
        int mSize;

        int mMutation;
        int mOptimalLevel;

        /**
         * A reference to the script function that is called each update.
         */
        Script::Ref mUpdateCallback;

        friend class MonsterManager;
        friend class MonsterComponent;
};

/**
 * The component for a fightable monster with its own AI
 */
class MonsterComponent : public Component
{
    public:
        static const ComponentType type = CT_Monster;

        MonsterComponent(Entity &entity, MonsterClass *);

        /**
         * Returns monster specy.
         */
        MonsterClass *getSpecy() const
        { return mSpecy; }

        /**
         * Performs one step of controller logic.
         */
        void update(Entity &entity);

        /**
         * Signal handler
         */
        void monsterDied(Entity *monster);

    private:
        static const int DECAY_TIME = 50;

        MonsterClass *mSpecy;

        /** Time until dead monster is removed */
        Timeout mDecayTimeout;
};

inline void MonsterClass::setAttribute(AttributeInfo *attribute, double value)
{
    mAttributes[attribute] = value;
}

inline const std::map<AttributeInfo *, double>
&MonsterClass::getAttributes() const
{
    return mAttributes;
}

inline void MonsterClass::addAbility(AbilityManager::AbilityInfo *info)
{
    mAbilities.insert(info);
}

inline const std::set<AbilityManager::AbilityInfo *>
&MonsterClass::getAbilities() const
{
    return mAbilities;
}

#endif // MONSTER_H
