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

#ifndef COLLISIONDETECTION_H
#define COLLISIONDETECTION_H

class Point;

/**
 * This namespace collects all needed collision detection functions
 */
namespace Collision
{

    bool circleWithCirclesector(const Point &circlePos, int circleRadius,
                                const Point &secPos, int secRadius,
                                float secAngle, float secSize);

    /**
     * Checks if a disk and a circle-sector collide
     *
     * @param halfTopAngle
     *        The half-top-angle of the circle sector in degrees (0,359).
     * @param placeAngle
     *        The placement-angle of the circle sector in degrees (0,359).
     */
    bool diskWithCircleSector(const Point &diskCenter, int diskRadius,
                              const Point &sectorCenter, int sectorRadius,
                              int halfTopAngle, int placeAngle);

    /**
     * Checks if two circles intersect.
     */
    bool circleWithCircle(const Point &center1, int radius1,
                          const Point &center2, int radius2);
}

#endif
