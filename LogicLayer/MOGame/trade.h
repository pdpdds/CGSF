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

#ifndef GAMESERVER_TRADE_H
#define GAMESERVER_TRADE_H

#include <vector>

#include "attributemanager.h"

class Entity;
class Inventory;

class Trade
{
    public:

        /**
         * Sets up a trade between two characters.
         * Asks for an acknowledgment from the second one.
         */
        Trade(Entity *, Entity *);

        /**
         * Cancels a trade by a given character (optional).
         * Warns the other character the trade is cancelled.
         * Takes care of cleaning afterwards.
         */
        void cancel();

        /**
         * Requests a trade to start with given public ID.
         * Continues the current trade if the ID is correct, cancels it
         * otherwise.
         * @return true if the current trade keeps going.
         */
        bool request(Entity *, int);

        /**
         * Confirm the trade.
         */
        void confirm(Entity *);

        /*
         * Agree to complete the trade
         */
        void agree(Entity *c);

        /**
         * Adds some items to the trade.
         */
        void addItem(Entity *, int slot, int amount);

        /**
         * Adds some money to the trade.
         */
        void setMoney(Entity *, int amount);

    private:

        ~Trade();

        struct TradedItem
        {
            unsigned id;
            unsigned slot;
            int amount;
        };

        typedef std::vector< TradedItem > TradedItems;

        /*
         * See trade.cpp for doc on TradeStates
         */
        enum TradeState
        {
            TRADE_INIT = 0,
            TRADE_RUN,
            TRADE_CONFIRM_WAIT,
            TRADE_CONFIRMED,
            TRADE_AGREE_WAIT
        };

        static bool perform(TradedItems items, Inventory &inv1, Inventory &inv2);

        Entity *mChar1, *mChar2;       /**< Characters involved. */
        TradedItems mItems1, mItems2; /**< Traded items. */
        int mMoney1, mMoney2;         /**< Traded money. */
        TradeState mState;            /**< State of transaction. */
        AttributeInfo *mCurrencyAttribute;
};

#endif
