/*
 *  The Mana Server
 *  Copyright (C) 2007-2010  The Mana World Development Team
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

#ifndef COMMON_INVENTORYDATA_H
#define COMMON_INVENTORYDATA_H

#include <vector>
#include <map>
#include <set>

/**
 * Numbers of inventory slots
 * TODO: Make this configurable and sent to the client.
 */
#define INVENTORY_SLOTS 50

/**
 * Structure storing an item in the inventory.
 */
struct InventoryItem
{
    InventoryItem():
        slot(0),
        itemId(0),
        amount(0),
        equipmentSlot(0)
    {}

    unsigned slot;
    unsigned itemId;
    unsigned amount;
    unsigned equipmentSlot; /** 0 if not equipped */
};

// inventory slot id -> { item }
typedef std::map< unsigned, InventoryItem > InventoryData;

// the slots which are equipped
typedef std::set<int> EquipData;

/**
 * Structure storing the equipment and inventory of a Player.
 */
struct Possessions
{
    friend class Inventory;
public:
    const EquipData &getEquipment() const
    { return equipment; }

    const InventoryData &getInventory() const
    { return inventory; }

    /**
     * Should be done only at character serialization and storage load time.
     */
    void setEquipment(EquipData &equipData)
    { equipment.swap(equipData); }
    void setInventory(InventoryData &inventoryData)
    { inventory.swap(inventoryData); }

private:
    InventoryData inventory;
    EquipData equipment;
};

#endif
