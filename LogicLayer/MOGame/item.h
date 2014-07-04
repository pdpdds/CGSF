/*
 *  The Mana Server
 *  Copyright (C) 2004-2010  The Mana World Development Team
 *  Copyright (C) 2010-2012  The Mana Developers
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

#ifndef ITEM_H
#define ITEM_H

#include <vector>

#include "actorcomponent.h"
#include "scripting/script.h"

class Entity;
class ItemClass;
class MapComposite;

// Indicates the equip slot "cost" to equip an item.
struct ItemEquipRequirement {
    ItemEquipRequirement():
        equipSlotId(0),
        capacityRequired(0)
    {}

    unsigned equipSlotId, capacityRequired;
};

enum ItemTriggerType
{
    ITT_NULL = 0,
    ITT_IN_INVY,    // Associated effects apply when the item is in the inventory
    ITT_ACTIVATE,   // Associated effects apply when the item is activated
    ITT_EQUIP,      // Associated effects apply when the item is equipped
    ITT_LEAVE_INVY, // Associated effects apply when the item leaves the inventory
    ITT_UNEQUIP,    // Associated effects apply when the item is unequipped
    ITT_EQUIPCHG    // When the item is still equipped, but in a different way
};

enum ItemEffectType
{
    // Effects that are removed automatically when the trigger ends
    // (ie. item no longer exists in invy, unequipped)
    IET_ATTR_MOD = 0, // Modify a given attribute with a given value
    // Effects that do not need any automatic removal
    IET_COOLDOWN,     // Set a cooldown to this item, preventing activation for n ticks
    IET_G_COOLDOWN,   // Set a cooldown to all items of this type for this being
    IET_SCRIPT        // Call an associated lua script with given variables
};

struct ItemTrigger
{
    ItemTriggerType apply;
    ItemTriggerType dispell;
};


class ItemEffectInfo
{
    public:
        virtual ~ItemEffectInfo() {}

        virtual bool apply(Entity *itemUser) = 0;
        virtual void dispell(Entity *itemUser) = 0;
};

class ItemEffectAttrMod : public ItemEffectInfo
{
    public:
        ItemEffectAttrMod(AttributeInfo *attribute,
                          unsigned layer, double value, unsigned modId,
                          unsigned duration = 0)
                : mAttribute(attribute)
                , mAttributeLayer(layer)
                , mMod(value)
                , mDuration(duration)
                , mModId(modId)
        {}

        bool apply(Entity *itemUser);
        void dispell(Entity *itemUser);

    private:
        AttributeInfo *mAttribute;
        unsigned mAttributeLayer;
        double mMod;
        unsigned mDuration;
        unsigned mModId;
};

class ItemEffectConsumes : public ItemEffectInfo
{
    public:
        bool apply(Entity *)
        { return true; }
        void dispell(Entity *)
        {}
};

class ItemEffectScript : public ItemEffectInfo
{
    public:
        ItemEffectScript(ItemClass *itemClass,
                         const std::string &activateEventName,
                         const std::string &dispellEventName):
            mItemClass(itemClass),
            mActivateEventName(activateEventName),
            mDispellEventName(dispellEventName)
        {}

        ~ItemEffectScript();

        bool apply(Entity *itemUser);
        void dispell(Entity *itemUser);

    private:
        ItemClass *mItemClass;
        std::string mActivateEventName;
        std::string mDispellEventName;
};


/**
 * Class for simple reference to item information.
 */
class ItemClass
{
    public:
        ItemClass(int id, unsigned maxperslot):
            mDatabaseID(id),
            mName("unnamed"),
            mSpriteID(0),
            mCost(0),
            mMaxPerSlot(maxperslot)
        {}

        ~ItemClass();

        /**
         * Returns the name of the item type
         */
        const std::string &getName() const
        { return mName; }

        /**
         * Sets the name of the item type
         */
        void setName(const std::string &name)
        { mName = name; }

        /**
         * Applies the modifiers of an item to a given user.
         * @return true if item should be removed.
         */
        bool useTrigger(Entity *itemUser, ItemTriggerType trigger);

        /**
         * Gets unit cost of these items.
         */
        int getCost() const
        { return mCost; }

        /**
         * Gets max item per slot.
         */
        unsigned getMaxPerSlot() const
        { return mMaxPerSlot; }

        bool hasTrigger(ItemTriggerType id)
        { return mEffects.count(id); }

        /**
         * Gets database ID.
         */
        int getDatabaseID() const
        { return mDatabaseID; }

        /**
         * Gets the sprite ID.
         * @note At present this is only a stub, and will always return zero.
         *       When you would want to extend serializeLooks to be more
         *       efficient, keep track of a sprite id here.
         */
        int getSpriteID() const
        { return mSpriteID; }

        /**
         * Returns equip requirement.
         */
        const ItemEquipRequirement &getItemEquipRequirement() const
        { return mEquipReq; }

        void setEventCallback(const std::string &event, Script *script)
        { script->assignCallback(mEventCallbacks[event]); }

        Script::Ref getEventCallback(const std::string &event) const
        { return mEventCallbacks.value(event); }

    private:
        /**
         * Add an effect to a trigger
         * @param effect  The effect to be run when the trigger is hit.
         * @param id      The trigger type.
         * @param dispell The trigger that the effect should be dispelled on.
         * @note  FIXME:  Should be more than one trigger that an effect
         *                can be dispelled from.
         */
        void addEffect(ItemEffectInfo *effect,
                       ItemTriggerType id,
                       ItemTriggerType dispell = ITT_NULL);

        unsigned short mDatabaseID; /**< Item reference information */
        std::string mName; /**< name used to identify the item class */
        /** The sprite that should be shown to the character */
        unsigned short mSpriteID;
        unsigned short mCost;     /**< Unit cost the item. */
        /** Max item amount per slot in inventory. */
        unsigned mMaxPerSlot;

        std::multimap< ItemTriggerType, ItemEffectInfo * > mEffects;
        std::multimap< ItemTriggerType, ItemEffectInfo * > mDispells;

        /**
         * Requirement for equipping.
         */
        ItemEquipRequirement mEquipReq;

        /**
         * Named event callbacks. Can be used in custom item effects.
         */
        utils::NameMap<Script::Ref> mEventCallbacks;

        friend class ItemManager;
};

/**
 * An item stack lying on the floor in the game world.
 */
class ItemComponent : public Component
{
    public:
        static const ComponentType type = CT_Item;

        ItemComponent(ItemClass *type, int amount);

        ItemClass *getItemClass() const
        { return mType; }

        int getAmount() const
        { return mAmount; }

        void update(Entity &entity);

    private:
        ItemClass *mType;
        unsigned char mAmount;
        int mLifetime;
};

namespace Item {

/**
 * @brief Creates an item actor.
 *
 * The returned actor should be inserted into the game state, usually with
 * either GameState::insertOrDelete or GameState::enqueueInsert.
 *
 * @param map       the map of the item
 * @param pos       the position of the item
 * @param itemClass the class of the item
 * @param amount    the amount of items on the stack
 *
 * @return the created item
 */
Entity *create(MapComposite *map, Point pos, ItemClass *itemClass, int amount);

} // namespace Item

#endif // ITEM_H
