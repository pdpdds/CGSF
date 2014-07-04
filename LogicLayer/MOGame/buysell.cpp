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

#include "buysell.h"

#include "charactercomponent.h"
#include "gamehandler.h"
#include "inventory.h"
#include "itemmanager.h"
#include "item.h"
#include "net/messageout.h"
#include "common/defines.h"

#include <algorithm>

BuySell::BuySell(Entity *c, bool sell)
    : mCurrency(attributeManager->getAttributeInfo(ATTR_GP))
    , mChar(c)
    , mSell(sell)
{
    c->getComponent<CharacterComponent>()->setBuySell(this);
}

BuySell::~BuySell()
{
    mChar->getComponent<CharacterComponent>()->setBuySell(nullptr);
}

void BuySell::cancel()
{
    delete this;
}

bool BuySell::registerItem(unsigned id, int amount, int cost)
{
    if (mSell)
    {
        int nb = Inventory(mChar).count(id);
        if (nb == 0)
            return false;
        if (!amount || nb < amount)
            amount = nb;
    }

    TradedItem item;
    item.itemId = id;
    item.amount = amount;
    item.cost = cost;
    mItems.push_back(item);
    return true;
}


int BuySell::registerPlayerItems()
{
    if (!mSell)
        return 0;

    int nbItemsToSell = 0;

    // We parse the player inventory and add all item
    // in a sell list.
    auto *component = mChar->getComponent<CharacterComponent>();
    const InventoryData &inventoryData =
            component->getPossessions().getInventory();
    for (InventoryData::const_iterator it = inventoryData.begin(),
        it_end = inventoryData.end(); it != it_end; ++it)
    {
        unsigned amount = it->second.amount;
        if (!amount)
            continue;

        unsigned id = it->second.itemId;
        int cost = -1;
        if (itemManager->getItem(id))
        {
            cost = itemManager->getItem(id)->getCost();
        }
        else
        {
            LOG_WARN("registerPlayersItems(): The character Id: "
                << mChar->getComponent<ActorComponent>()->getPublicID()
                << " has unknown items (Id: " << id
                << "). They have been ignored.");
            continue;
        }

        if (cost < 1)
            continue;

        // We check if the item Id has been already
        // added. If so, we cumulate the amounts.
        bool itemAlreadyAdded = false;
        for (TradedItems::iterator i = mItems.begin(),
            i_end = mItems.end(); i != i_end; ++i)
        {
            if (i->itemId == id)
            {
                itemAlreadyAdded = true;
                i->amount += amount;
                break;
            }
        }

        if (!itemAlreadyAdded)
        {
            TradedItem tradeItem;
            tradeItem.itemId = id;
            tradeItem.amount = amount;
            tradeItem.cost = cost;
            mItems.push_back(tradeItem);
            nbItemsToSell++;
        }
    }
    return nbItemsToSell;
}

bool BuySell::start(Entity *actor)
{
    if (mItems.empty())
    {
        cancel();
        return false;
    }

    MessageOut msg(mSell ? GPMSG_NPC_SELL : GPMSG_NPC_BUY);
    msg.writeInt16(actor->getComponent<ActorComponent>()->getPublicID());
    for (TradedItems::const_iterator i = mItems.begin(),
         i_end = mItems.end(); i != i_end; ++i)
    {
        msg.writeInt16(i->itemId);
        msg.writeInt16(i->amount);
        msg.writeInt16(i->cost);
    }
    mChar->getComponent<CharacterComponent>()->getClient()->send(msg);
    return true;
}

void BuySell::perform(unsigned id, int amount)
{
    MessageOut msg(GPMSG_NPC_BUYSELL_RESPONSE);

    for (TradedItems::iterator i = mItems.begin(),
         i_end = mItems.end(); i != i_end; ++i)
    {
        if (i->itemId != id)
            continue;

        Inventory inv(mChar);

        auto *beingComponent = mChar->getComponent<BeingComponent>();
        const double currentMoney = beingComponent->getAttributeBase(mCurrency);

        if (i->amount && i->amount <= amount)
            amount = i->amount;

        if (mSell)
        {
            amount -= inv.remove(id, amount);
            beingComponent->setAttribute(*mChar, mCurrency,
                                         currentMoney + amount * i->cost);
        }
        else
        {
            amount = min(amount, ((int)currentMoney) / i->cost);
            amount -= inv.insert(id, amount);
            beingComponent->setAttribute(*mChar, mCurrency,
                                         currentMoney - amount * i->cost);
        }

        if (i->amount)
        {
            i->amount -= amount;
            if (!i->amount)
            {
                mItems.erase(i);
            }
        }

        msg.writeInt8(ERRMSG_OK);
        msg.writeInt16(id);
        msg.writeInt16(amount);
        mChar->getComponent<CharacterComponent>()->getClient()->send(msg);
        return;
    }

    msg.writeInt8(ERRMSG_FAILURE);
    mChar->getComponent<CharacterComponent>()->getClient()->send(msg);
}
