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

#include "timeout.h"

#include "state.h"

void Timeout::set(int ticks)
{
    mReference = GameState::getCurrentTick() + ticks;
}

void Timeout::setSoft(int ticks)
{
    int time = GameState::getCurrentTick();
    int newReference = time + ticks;
    if (mReference < time || mReference < newReference)
        mReference = newReference;
}

int Timeout::remaining() const
{
    return mReference - GameState::getCurrentTick();
}
