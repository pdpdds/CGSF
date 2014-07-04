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

#ifndef GAMESERVER_BUYSELL_H
#define GAMESERVER_BUYSELL_H

#include <vector>

#include "attributemanager.h"

class Entity;

class BuySell
{
    public:

        /**
         * Sets up a trade between a character and an NPC.
         */
        BuySell(Entity *, bool sell);

        /**
         * Cancels the trade.
         */
        void cancel();

        /**
         * Registers an item and indicates how many the NPC is ready to trade
         * and how much it will cost.
         * @return true if at least one item was registered.
         */
        bool registerItem(unsigned id, int amount, int cost);

        /**
         * Registers every player's item at an average cost given by the ItemDB.
         * @return the number of different soldable items.
         */
        int registerPlayerItems();

        /**
         * Sends the item list to player.
         * @return true if at least one item was registered before start.
         */
        bool start(Entity *actor);

        /**
         * Performs the trade.
         */
        void perform(unsigned id, int amount);

    private:

        ~BuySell();

        struct TradedItem
        {
            unsigned itemId;
            int amount;
            int cost;
        };

        typedef std::vector< TradedItem > TradedItems;

        /** The attribute ID of the currency to use. Hardcoded for now (FIXME) */
        AttributeInfo *mCurrency;

        Entity *mChar;      /**< Character involved. */
        TradedItems mItems; /**< Traded items. */
        bool mSell;         /**< Are items sold? */
};

#endif
