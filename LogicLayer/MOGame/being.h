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

#ifndef BEING_H
#define BEING_H

#include <string>
#include <vector>
#include <list>
#include <map>
#include "limits.h"

#include "actorcomponent.h"
#include "attribute.h"
#include "attributemanager.h"
#include "timeout.h"

#include "scripting/script.h"

class BeingComponent;
class MapComposite;
class StatusEffect;

typedef std::map<AttributeInfo *, Attribute> AttributeMap;

struct Status
{
    StatusEffect *status;
    unsigned time;  // Number of ticks
};

typedef std::map< int, Status > StatusEffects;

/**
 * Type definition for a list of hits
 */
typedef std::vector<unsigned> Hits;

/**
 * Generic being (living actor). Keeps direction, destination and a few other
 * relevant properties. Used for characters & monsters (all animated objects).
 */
class BeingComponent : public Component
{
    public:
        static const ComponentType type = CT_Being;

        /**
         * Proxy constructor.
         */
        BeingComponent(Entity &entity);

        /**
         * Update being state.
         */
        virtual void update(Entity &entity);

        /** Restores all hit points of the being */
        void heal(Entity &entity);

        /** Restores a specific number of hit points of the being */
        void heal(Entity &entity, int hp);

        /**
         * Changes status and calls all the "died" listeners.
         */
        virtual void died(Entity &entity);

        /**
         * Gets the destination coordinates of the being.
         */
        const Point &getDestination() const
        { return mDst; }

        /**
         * Sets the destination coordinates of the being.
         */
        void setDestination(Entity &entity, const Point &dst);

        /**
         * Sets the destination coordinates of the being to the current
         * position.
         */
        void clearDestination(Entity &entity);

        /**
         * Gets the old coordinates of the being.
         */
        const Point &getOldPosition() const
        { return mOld; }

        /**
         * Sets the facing direction of the being.
         */
        void setDirection(Entity &entity, BeingDirection direction);

        BeingDirection getDirection() const
        { return mDirection; }
        /**
         * Sets the current action.
         */
        void setAction(Entity &entity, BeingAction action);

        /**
         * Sets the current action.
         */
        BeingAction getAction() const
        { return mAction; }

        /**
         * Moves the being toward its destination.
         */
        void move(Entity &entity);

        /**
         * Returns the path to the being's current destination.
         */
        virtual Path findPath(Entity &);

        /** Gets the gender of the being (male or female). */
        BeingGender getGender() const
        { return mGender; }

        /** Sets the gender of the being (male or female). */
        void setGender(BeingGender gender);

        /**
         * Sets an attribute.
         */
        void setAttribute(Entity &entity, AttributeInfo *, double value);

        /**
         * Creates an Attribute that did not exist before
         *
         * @param id The id of the attribute
         * @param attributeInfo The info that describes the attribute
         */
        void createAttribute(AttributeInfo *);

        /**
         * Gets an attribute or 0 if not existing.
         */
        const Attribute *getAttribute(AttributeInfo *) const;

        const AttributeMap &getAttributes() const
        { return mAttributes; }

        /**
         * Gets an attribute base.
         */
        double getAttributeBase(AttributeInfo *) const;

        /**
         * Gets an attribute after applying modifiers.
         */
        double getModifiedAttribute(AttributeInfo *) const;

        /**
         * Checks whether or not an attribute exists in this being.
         * @returns True if the attribute is present in the being, false otherwise.
         */

        bool checkAttributeExists(AttributeInfo *attribute) const
        { return mAttributes.count(attribute); }

        /**
         * Adds a modifier to one attribute.
         * @param duration If non-zero, creates a temporary modifier that
         *        expires after \p duration ticks.
         * @param lvl If non-zero, indicates that a temporary modifier can be
         *        dispelled prematuraly by a spell of given level.
         */
        void applyModifier(Entity &entity, AttributeInfo *,
                           double value, unsigned layer,
                           unsigned duration = 0, unsigned id = 0);

        bool removeModifier(Entity &entity, AttributeInfo *,
                            double value, unsigned layer,
                            unsigned id = 0, bool fullcheck = false);

        /**
         * Called when an attribute modifier is changed.
         * Recalculate the base value of an attribute and update derived
         *     attributes if it has changed.
         * @returns Whether it was changed.
         */
        void recalculateBaseAttribute(Entity &,
                                      AttributeInfo *);

        /**
         * Attribute has changed, recalculate base value of dependant
         *     attributes (and handle other actions for the modified
         *     attribute)
         */
        void updateDerivedAttributes(Entity &entity,
                                     AttributeInfo *);

        /**
         * Sets a statuseffect on this being
         */
        void applyStatusEffect(int id, int time);

        /**
         * Removes the status effect
         */
        void removeStatusEffect(int id);

        /**
         * Returns true if the being has a status effect
         */
        bool hasStatusEffect(int id) const;

        const StatusEffects &getStatusEffects() const
        { return mStatus; }

        /**
         * Returns the time of the status effect if in effect, or 0 if not
         */
        unsigned getStatusEffectTime(int id) const;

        /**
         * Changes the time of the status effect (if in effect)
         */
        void setStatusEffectTime(int id, int time);

        /** Gets the name of the being. */
        const std::string &getName() const
        { return mName; }

        /** Sets the name of the being. */
        void setName(const std::string &name)
        { mName = name; }

        /**
         * Converts a direction to an angle. Used for combat hit checks.
         */
        static int directionToAngle(int direction);

        static void setUpdateDerivedAttributesCallback(Script *script)
        { script->assignCallback(mRecalculateDerivedAttributesCallback); }

        static void setRecalculateBaseAttributeCallback(Script *script)
        { script->assignCallback(mRecalculateBaseAttributeCallback); }

        sigc::signal<void, Entity *> signal_died;
        sigc::signal<void, Entity *, AttributeInfo *> signal_attribute_changed;

        /**
         * Activate an emote flag on the being.
         */
        void triggerEmote(Entity &entity, int id);

        /**
         * Tells the last emote used.
         */
        int getLastEmote() const
        { return mEmoteId; }

        /**
         * Update the being direction when moving so avoid directions desyncs
         * with other clients.
         */
        void updateDirection(Entity &entity,
                             const Point &currentPos,
                             const Point &destPos);

        void addHitTaken(unsigned damage);
        const Hits &getHitsTaken() const;
        void clearHitsTaken();

    protected:
        static const int TICKS_PER_HP_REGENERATION = 100;

        /** Delay until move to next tile in miliseconds. */
        unsigned short mMoveTime;
        BeingAction mAction;
        AttributeMap mAttributes;
        StatusEffects mStatus;
        Point mOld;                 /**< Old coordinates. */
        Point mDst;                 /**< Target coordinates. */
        BeingGender mGender;        /**< Gender of the being. */

    private:
        /**
         * Connected to signal_inserted to reset the old position.
         */
        void inserted(Entity *);

        Path mPath;
        BeingDirection mDirection;   /**< Facing direction. */

        std::string mName;

        /** Time until hp is regenerated again */
        Timeout mHealthRegenerationTimeout;

        /** The last being emote Id. Used when triggering a being emoticon. */
        int mEmoteId;

        Hits mHitsTaken;            //List of punches taken since last update.

        /** Called when derived attributes need to get calculated */
        static Script::Ref mRecalculateDerivedAttributesCallback;

        /** Called when a base attribute needs to get calculated */
        static Script::Ref mRecalculateBaseAttributeCallback;
};


inline void BeingComponent::addHitTaken(unsigned damage)
{
    mHitsTaken.push_back(damage);
}

/**
 * Gets the damage list.
 */
inline const Hits &BeingComponent::getHitsTaken() const
{
    return mHitsTaken;
}

/**
 * Clears the damage list.
 */
inline void BeingComponent::clearHitsTaken()
{
    mHitsTaken.clear();
}

#endif // BEING_H
