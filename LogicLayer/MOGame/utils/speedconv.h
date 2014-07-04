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

#ifndef SPEEDCONV_H
#define SPEEDCONV_H

// Simple helper functions for converting between tiles per
// second and the internal speed representation

namespace utils {
    /**
     * Translate the speed in tiles per second (tps)
     * into the raw speed used internally.
     * The raw speed is the number of tiles moves per server tick * 100
     * since a server tick is currently 100 ms.
     *
     * @param tps The speed value in tiles per second
     *
     * @returns The speed value in the internal representation
     */
    double tpsToRawSpeed(double);

    /**
     * Translate the raw speed used internally into a tile per second one.
     *
     * @param speed The speed value in the internal representation
     *
     * @returns The speed value in tiles per second
     */
    double rawSpeedToTps(double);
}

#endif // SPEEDCONV_H
