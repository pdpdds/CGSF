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

#ifndef INVENTORY_H
#define INVENTORY_H

#include "charactercomponent.h"
#include "net/messageout.h"

class ItemClass;

/**
 * Class used to handle Character possessions and prepare outgoing messages.
 */
class Inventory
{
    public:

        /**
         * Creates a view on the possessions of a character.
         */
        explicit Inventory(Entity *);
        Inventory(Entity *, Possessions &possessions);

        /**
         * Commits delayed changes if applicable.
         * Sends the update message to the client.
         */
        ~Inventory()
        {}

        /**
         * Sends complete inventory status to the client.
         */
        void sendFull() const;

        /**
         * Ensures the inventory is sane and apply equipment modifiers.
         * Should be run only once and the very first time.
         */
        void initialize();

        /**
         * Equips item from given inventory slot.
         * @param inventorySlot The slot in which the target item is in.
         * @returns whether the item could be equipped.
         */
        bool equip(int inventorySlot);

        /**
         * Unequips all the items with the given item id
         * @param itemId The item Id to unequip.
         * @returns whether all item id could be unequipped.
         * @note returns true when no item with given ids were equipped.
         */
        bool unequipAll(unsigned itemId);

        /**
         * Unequips item from given equipment slot.
         * @param itemSlot The item slot used to know what to unequip
         * @returns Whether it was unequipped.
         */
        bool unequip(unsigned itemSlot);

        /**
         * Inserts some items into the inventory.
         * @return number of items not inserted (to be dropped on floor?).
         */
        unsigned insert(unsigned itemId, unsigned amount);

        /**
         * Removes some items from inventory.
         * @return number of items not removed.
         */
        unsigned remove(unsigned itemId, unsigned amount);

        /**
         * Removes some items from inventory.
         * @return number of items not removed.
         */
        unsigned removeFromSlot(unsigned slot, unsigned amount);

        /**
         * Counts number of items with given Id.
         * @param itemId The id to look for.
         */
        unsigned count(unsigned itemId) const;

        /**
         * Gets the ID of the items in a given slot.
         */
        unsigned getItem(unsigned slot) const;

        /**
         * Returns the first inventory slot with the given item Id.
         * Returns -1 otherwise.
         */
        int getFirstSlot(unsigned itemId);

    private:
        /**
         * Tell whether the equipment slot has enough room in an equipment slot.
         * @param equipmentSlot the slot in equipement to check.
         * @param capacityRequested the capacity needed.
         */
        bool checkEquipmentCapacity(unsigned equipmentSlot,
                                    unsigned capacityRequested);

        /**
         * Test whether the inventory has enough space to welcome
         * the willing-to-be equipment slot.
         * @todo
         */
        bool hasInventoryEnoughSpace(unsigned /* equipmentSlot */)
        { return true; }

        /**
         * Test the items unequipment requirements.
         * This is especially useful for scripted equipment.
         * @todo
         */
        bool testUnequipScriptRequirements(unsigned /* equipementSlot */)
        { return true; }

        /**
         * Test the items equipment for scripted requirements.
         * @todo
         */
        bool testEquipScriptRequirements(unsigned /* itemId */)
        { return true; }

        /**
         * Check the inventory is within the slot limit and capacity.
         * Forcibly delete items from the end if it is not.
         * @todo Drop items instead?
         */
        void checkInventorySize();

        /**
         * Check potential visible character sprite changes.
         */
        void checkLookchanges(unsigned slotTypeId);

        /**
         * Apply equipment triggers.
         */
        void updateEquipmentTrigger(unsigned oldId, unsigned itemId);
        void updateEquipmentTrigger(ItemClass *oldI, ItemClass *newI);

        Possessions *mPoss; /**< Pointer to the modified possessions. */

        Entity *mCharacter; /**< Character to notify. */
};

#endif
