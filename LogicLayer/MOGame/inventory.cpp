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

#include <algorithm>
#include <cassert>

#include "gamehandler.h"
#include "inventory.h"
#include "item.h"
#include "itemmanager.h"
#include "state.h"
#include "net/messageout.h"
#include "utils/logger.h"

Inventory::Inventory(Entity *p):
    mPoss(&p->getComponent<CharacterComponent>()->getPossessions()),
    mCharacter(p)
{
}

Inventory::Inventory(Entity *p, Possessions &possessions):
    mPoss(&possessions),
    mCharacter(p)
{
}

void Inventory::sendFull() const
{
    /* Sends all the information needed to construct inventory
       and equipment to the client */
    MessageOut m(GPMSG_INVENTORY_FULL);

    m.writeInt16(mPoss->inventory.size());
    for (InventoryData::const_iterator it = mPoss->inventory.begin(),
         l_end = mPoss->inventory.end(); it != l_end; ++it)
    {
        assert(it->second.itemId);
        m.writeInt16(it->first); // slot
        m.writeInt16(it->second.itemId);
        m.writeInt16(it->second.amount);
        m.writeInt16(it->second.equipmentSlot);
    }

    gameHandler->sendTo(mCharacter, m);
}

void Inventory::initialize()
{
    /*
     * Set the equipment slots
     */
    for (EquipData::iterator it = mPoss->equipment.begin(),
            it_end = mPoss->equipment.end(); it != it_end; ++it)
    {
        InventoryData::iterator itemIt = mPoss->inventory.find(*it);
        const ItemEquipRequirement &equipReq = itemManager->getItem(
                itemIt->second.itemId)->getItemEquipRequirement();
        itemIt->second.equipmentSlot = equipReq.equipSlotId;
    }

    /*
     * Construct a set of item Ids to keep track of duplicate item Ids.
     */
    std::set<unsigned> itemIds;
    std::set<unsigned> equipmentIds;

    /*
     * Construct a set of itemIds to keep track of duplicate itemIds.
     */
    for (InventoryData::iterator it = mPoss->inventory.begin(),
         it_end = mPoss->inventory.end(); it != it_end;)
    {
        ItemClass *item = itemManager->getItem(it->second.itemId);
        if (item)
        {
            // If the insertion succeeded, it's the first time we're
            // adding the item in the inventory. Hence, we can trigger
            // item presence in inventory effect.
            if (itemIds.insert(it->second.itemId).second)
                item->useTrigger(mCharacter, ITT_IN_INVY);

            if (it->second.equipmentSlot != 0 &&
                equipmentIds.insert(it->second.itemId).second)
            {
                item->useTrigger(mCharacter, ITT_EQUIP);
            }
            ++it;
        }
        else
        {
            LOG_WARN("Equipment: deleting unknown item id "
                     << it->second.itemId << " from the equipment of '"
                     << mCharacter->getComponent<BeingComponent>()->getName()
                     << "'!");
            mPoss->inventory.erase(it++);
        }
    }
}

unsigned Inventory::getItem(unsigned slot) const
{
    InventoryData::iterator item = mPoss->inventory.find(slot);
    return item != mPoss->inventory.end() ? item->second.itemId : 0;
}

unsigned Inventory::insert(unsigned itemId, unsigned amount)
{
    if (!itemId || !amount)
        return 0;

    MessageOut invMsg(GPMSG_INVENTORY);
    ItemClass *item = itemManager->getItem(itemId);
    if (!item) {
        LOG_ERROR("Inventory: Trying to insert invalid item id " << itemId
                  << " (amount: " << amount << ")");
        return amount;
    }
    unsigned maxPerSlot = item->getMaxPerSlot();

    LOG_DEBUG("Inventory: Inserting " << amount << " item(s) Id: " << itemId
              << " for character '"
              << mCharacter->getComponent<BeingComponent>()->getName() << "'.");

    InventoryData::iterator it, it_end = mPoss->inventory.end();
    // Add to slots with existing items of this type first.
    for (it = mPoss->inventory.begin(); it != it_end; ++it)
    {
        if (it->second.itemId == itemId)
        {
            // If the slot is full, try the next slot
            if (it->second.amount >= maxPerSlot)
                continue;

            // Add everything that'll fit to the stack
            unsigned short spaceLeft = maxPerSlot - it->second.amount;
            if (spaceLeft >= amount)
            {
                it->second.amount += amount;
                amount = 0;
                LOG_DEBUG("Everything inserted at slot id: " << it->first);
            }
            else
            {
                it->second.amount += spaceLeft;
                amount -= spaceLeft;
                LOG_DEBUG(spaceLeft << " item(s) inserted at slot id: "
                          << it->first);
            }

            invMsg.writeInt16(it->first);
            invMsg.writeInt16(itemId);
            invMsg.writeInt16(it->second.amount);
            if (!amount)
                break;
        }
    }

    int slot = 0;
    // We still have some left, so add to blank slots.
    for (it = mPoss->inventory.begin();; ++it)
    {
        if (!amount)
            break;
        int lim = (it == it_end) ? INVENTORY_SLOTS : it->first;
        while (amount && slot < lim)
        {
            int additions = std::min(amount, maxPerSlot);
            mPoss->inventory[slot].itemId = itemId;
            mPoss->inventory[slot].amount = additions;
            amount -= additions;
            LOG_DEBUG(additions << " item(s) inserted at slot id: " << slot);
            invMsg.writeInt16(slot++); // Last read, so also increment
            invMsg.writeInt16(itemId);
            invMsg.writeInt16(additions);
        }
        ++slot; // Skip the slot that the iterator points to
        if (it == it_end)
            break;
    }

    item->useTrigger(mCharacter, ITT_IN_INVY);

    // Send that first, before checking potential removals
    if (invMsg.getLength() > 2)
        gameHandler->sendTo(mCharacter, invMsg);

    return amount;
}

unsigned Inventory::count(unsigned itemId) const
{
    unsigned nb = 0;
    for (InventoryData::iterator it = mPoss->inventory.begin(),
        it_end = mPoss->inventory.end(); it != it_end; ++it)
    {
        if (it->second.itemId == itemId)
            nb += it->second.amount;
    }

    return nb;
}

int Inventory::getFirstSlot(unsigned itemId)
{
    for (InventoryData::iterator it = mPoss->inventory.begin(),
        it_end = mPoss->inventory.end(); it != it_end; ++it)
        if (it->second.itemId == itemId)
            return (int)it->first;
    return -1;
}

unsigned Inventory::remove(unsigned itemId, unsigned amount)
{
    if (!itemId || !amount)
        return amount;

    LOG_DEBUG("Inventory: Request remove of " << amount << " item(s) id: "
              << itemId << " for character: '"
              << mCharacter->getComponent<BeingComponent>()->getName()
              << "'.");

    MessageOut invMsg(GPMSG_INVENTORY);
    bool triggerLeaveInventory = true;
    for (InventoryData::iterator it = mPoss->inventory.begin();
         it != mPoss->inventory.end();)
    {
        LOG_DEBUG("Remove: Treating slot id: " << it->first);
        if (it->second.itemId == itemId)
        {
            if (amount)
            {
                unsigned sub = std::min(amount, it->second.amount);
                amount -= sub;
                it->second.amount -= sub;
                invMsg.writeInt16(it->first);
                if (it->second.amount)
                {
                    invMsg.writeInt16(it->second.itemId);
                    invMsg.writeInt16(it->second.amount);
                    // Some still exist, and we have none left to remove, so
                    // no need to run leave invy triggers.
                    if (!amount)
                        triggerLeaveInventory = false;
                    LOG_DEBUG("Slot id: " << it->first << " has now "
                              << it->second.amount << "item(s).");
                }
                else
                {
                    invMsg.writeInt16(0);
                    // Ensure the slot is set empty.
                    LOG_DEBUG("Slot id: " << it->first << " is now empty.");
                    mPoss->inventory.erase(it++);
                    continue;
                }
            }
            else
            {
                // We found an instance of them existing and have none left to
                // remove, so no need to run leave invy triggers.
                triggerLeaveInventory = false;
            }
        }
        ++it;
    }

    if (triggerLeaveInventory)
        itemManager->getItem(itemId)->useTrigger(mCharacter, ITT_LEAVE_INVY);

    if (invMsg.getLength() > 2)
        gameHandler->sendTo(mCharacter, invMsg);

    return amount;
}

unsigned Inventory::removeFromSlot(unsigned slot, unsigned amount)
{
    InventoryData::iterator it = mPoss->inventory.find(slot);

    // When the given slot doesn't exist, we can't remove anything
    if (it == mPoss->inventory.end())
        return amount;

    LOG_DEBUG("Inventory: Request Removal of " << amount << " item(s) in slot: "
              << slot << " for character: '"
              << mCharacter->getComponent<BeingComponent>()->getName() << "'.");

    MessageOut invMsg(GPMSG_INVENTORY);
    // Check if an item of the same id exists elsewhere in the inventory
    bool exists = false;
    for (InventoryData::const_iterator it2 = mPoss->inventory.begin(),
         it2_end = mPoss->inventory.end();
         it2 != it2_end; ++it2)
    {
        if (it2->second.itemId == it->second.itemId
                && it->first != it2->first)
        {
            exists = true;
            break;
        }
    }

    // We check whether it's the last slot where we can find that item id.
    bool lastSlotOfItemRemaining = false;
    if (!exists && it->second.itemId)
        lastSlotOfItemRemaining = true;

    unsigned sub = std::min(amount, it->second.amount);
    amount -= sub;
    it->second.amount -= sub;
    invMsg.writeInt16(it->first);
    if (it->second.amount)
    {
        invMsg.writeInt16(it->second.itemId);
        invMsg.writeInt16(it->second.amount);
    }
    else
    {
        invMsg.writeInt16(0);

        // The item(s) was(were) the last one(s) in the inventory.
        if (lastSlotOfItemRemaining)
        {
            if (ItemClass *ic = itemManager->getItem(it->second.itemId))
                ic->useTrigger(mCharacter, ITT_LEAVE_INVY);
        }
        mPoss->inventory.erase(it);
    }

    if (invMsg.getLength() > 2)
        gameHandler->sendTo(mCharacter, invMsg);

    return amount;
}


void Inventory::updateEquipmentTrigger(unsigned oldId, unsigned newId)
{
    if (!oldId && !newId)
        return;
    updateEquipmentTrigger(oldId ? itemManager->getItem(oldId) : 0,
                    newId ? itemManager->getItem(newId) : 0);
}

void Inventory::updateEquipmentTrigger(ItemClass *oldI, ItemClass *newI)
{
    // This should only be called when applying changes, either directly
    // in non-delayed mode or when the changes are committed in delayed mode.
    if (!oldI && !newI)
        return;
    if (oldI && newI)
        oldI->useTrigger(mCharacter, ITT_EQUIPCHG);
    else if (oldI)
        oldI->useTrigger(mCharacter, ITT_UNEQUIP);
    else if (newI)
        newI->useTrigger(mCharacter, ITT_EQUIP);
}

bool Inventory::checkEquipmentCapacity(unsigned equipmentSlot,
                                       unsigned capacityRequested)
{
    int capacity = itemManager->getEquipSlotCapacity(equipmentSlot);

    // If the equipement slot doesn't exist, we can't equip on it.
    if (capacity <= 0)
        return false;

    // Test whether the slot capacity requested is reached.
    for (EquipData::const_iterator it = mPoss->equipment.begin(),
         it_end = mPoss->equipment.end(); it != it_end; ++it)
    {
        InventoryData::iterator itemIt = mPoss->inventory.find(*it);
        if (itemIt->second.equipmentSlot == equipmentSlot)
        {
            const int itemId = itemIt->second.itemId;
            const ItemClass *item = itemManager->getItem(itemId);
            capacity -= item->getItemEquipRequirement().capacityRequired;
        }
    }

    assert(capacity >= 0); // A should never happen case.

    if (capacity < (int)capacityRequested)
        return false;

    return true;
}

bool Inventory::equip(int inventorySlot)
{
    // Test inventory slot existence
    InventoryData::iterator itemIt;
    if ((itemIt = mPoss->inventory.find(inventorySlot)) == mPoss->inventory.end())
    {
        LOG_DEBUG("No existing item in inventory at slot: " << inventorySlot);
        return false;
    }

    InventoryItem &item = itemIt->second;

    // Already equipped?
    if (item.equipmentSlot != 0)
        return false;

    // Test the equipment scripted requirements
    if (!testEquipScriptRequirements(item.itemId))
        return false;

    // Test the equip requirements. If none, it's not an equipable item.
    const ItemEquipRequirement &equipReq =
        itemManager->getItem(item.itemId)->getItemEquipRequirement();
    if (!equipReq.equipSlotId)
    {
        LOG_DEBUG("No equip requirements for item id: " << item.itemId
            << " at slot: " << inventorySlot);
        return false;
    }

    // List of potential unique itemInstances to unequip first.
    std::set<unsigned> slotsToUnequipFirst;

    // We first check the equipment slots for:
    // - 1. whether enough total equip slot space is available.
    // - 2. whether some other equipment is to be unequipped first.

    // If not enough total space in the equipment slot is available,
    // we cannot equip.
    if (itemManager->getEquipSlotCapacity(equipReq.equipSlotId)
            < equipReq.capacityRequired)
    {
        LOG_DEBUG("Not enough equip capacity at slot: " << equipReq.equipSlotId
                  << ", total available: "
                  << itemManager->getEquipSlotCapacity(equipReq.equipSlotId)
                  << ", required: " << equipReq.capacityRequired);
        return false;
    }

    // Test whether some item(s) is(are) to be unequipped first.
    if (!checkEquipmentCapacity(equipReq.equipSlotId,
                                equipReq.capacityRequired))
    {
        // And test whether the unequip action would succeed first.
        if (testUnequipScriptRequirements(equipReq.equipSlotId)
            && hasInventoryEnoughSpace(equipReq.equipSlotId))
        {
            // Then, we unequip each iteminstance of the equip slot
            for (EquipData::iterator it = mPoss->equipment.begin(),
                 it_end = mPoss->equipment.end(); it != it_end; ++it)
            {
                const unsigned slot = *it;
                InventoryData::iterator itemIt = mPoss->inventory.find(slot);
                assert(itemIt != mPoss->inventory.end());
                if (itemIt->second.equipmentSlot == equipReq.equipSlotId) {
                    slotsToUnequipFirst.insert(itemIt->first);
                }
            }
        }
        else
        {
            // Some non-unequippable equipment is to be unequipped first.
            // Can be the case of cursed items,
            // or when the inventory is full, for instance.
            return false;
        }
    }

    // Potential Pre-unequipment process
    for (std::set<unsigned>::const_iterator itemsToUnequip =
         slotsToUnequipFirst.begin(),
         itemsToUnequip_end = slotsToUnequipFirst.end();
         itemsToUnequip != itemsToUnequip_end; ++itemsToUnequip)
    {
        if (!unequip(*itemsToUnequip))
        {
            // Something went wrong even when we tested the unequipment process.
            LOG_WARN("Unable to unequip even when unequip was tested. "
                     "Character : "
                     << mCharacter->getComponent<BeingComponent>()->getName()
                     << ", unequip slot: " << *itemsToUnequip);
            return false;
        }
    }

    // Actually equip the item now that the requirements has met.
    item.equipmentSlot = equipReq.equipSlotId;
    mPoss->equipment.insert(inventorySlot);

    MessageOut equipMsg(GPMSG_EQUIP);
    equipMsg.writeInt16(inventorySlot);
    equipMsg.writeInt16(item.equipmentSlot);
    gameHandler->sendTo(mCharacter, equipMsg);

    // New item trigger
    updateEquipmentTrigger(0, item.itemId);

    // Update look when necessary
    checkLookchanges(equipReq.equipSlotId);

    return true;
}

bool Inventory::unequipAll(unsigned itemId)
{
    while (true)
    {
        const int slot = getFirstSlot(itemId);
        // No item left
        if (slot == -1)
            return true;

        if (!unequip(slot))
            return false;
    }

    // silence compiler warnings
    assert(false);
    return false;
}

bool Inventory::unequip(unsigned itemSlot)
{
    InventoryData::iterator it = mPoss->inventory.find(itemSlot);
    if (it == mPoss->inventory.end())
    {
        LOG_DEBUG("Tried to unequip invalid item at slot " << itemSlot);
        return false;
    }

    InventoryItem &item = it->second;

    // Item was not equipped
    if (item.equipmentSlot == 0)
        return false;

    const unsigned slotTypeId = item.equipmentSlot;

    // unequip
    item.equipmentSlot = 0;
    mPoss->equipment.erase(mPoss->equipment.find(itemSlot));

    MessageOut equipMsg(GPMSG_UNEQUIP);
    equipMsg.writeInt16(itemSlot);
    gameHandler->sendTo(mCharacter, equipMsg);

    // Apply unequip trigger
    updateEquipmentTrigger(item.itemId, 0);

    checkLookchanges(slotTypeId);

    return true;
}

void Inventory::checkLookchanges(unsigned slotTypeId)
{
    if (itemManager->isEquipSlotVisible(slotTypeId))
        mCharacter->getComponent<ActorComponent>()->raiseUpdateFlags(
                UPDATEFLAG_LOOKSCHANGE);
}
