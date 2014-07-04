/*
 *  The Mana Server
 *  Copyright (C) 2011  The Mana Development Team
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

#ifndef FLOOR_ITEM_H
#define FLOOR_ITEM_H

class FloorItem
{
public:
    FloorItem():
        mItemId(0), mItemAmount(0), mPosX(0), mPosY(0)
    {}

    FloorItem(int itemId, int itemAmount, int posX, int posY):
        mItemId(itemId), mItemAmount(itemAmount), mPosX(posX), mPosY(posY)
    {}

    /**
     * Returns the item id
     */
    int getItemId() const
    { return mItemId; }

    /**
     * Returns the amount of items
     */
    int getItemAmount() const
    { return mItemAmount; }

    /**
     * Returns the position x of the item(s)
     */
    int getPosX() const
    { return mPosX; }

    /**
     * Returns the position x of the item(s)
     */
    int getPosY() const
    { return mPosY; }

private:
    int mItemId;
    int mItemAmount;
    int mPosX;
    int mPosY;
};

#endif // FLOOR_ITEM_H
