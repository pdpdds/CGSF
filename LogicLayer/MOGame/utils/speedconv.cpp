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

#include "utils/speedconv.h"

#include "common/defines.h"

// Defines the max base scale used to compute the raw speed system.
// The raw speed is the number of tile moves per server tick multiplied the
// server tick value in miliseconds.
#define MAX_MOVE_TIME 320

double utils::tpsToRawSpeed(double tps)
{
    return ((MAX_MOVE_TIME * WORLD_TICK_MS) / (tps * DEFAULT_TILE_LENGTH));
}

double utils::rawSpeedToTps(double speed)
{
    return ((MAX_MOVE_TIME * WORLD_TICK_MS) / (speed * DEFAULT_TILE_LENGTH));
}
