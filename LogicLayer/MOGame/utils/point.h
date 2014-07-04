/*
 *  The Mana Server
 *  Copyright (C) 2004-2011  The Mana World Development Team
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

#ifndef POINT_H
#define POINT_H

#include <algorithm>
#include <ostream>

/**
 * A point. Usually represents pixel coordinates on a map.
 */
class Point
{
    public:
        Point():
            x(0), y(0)
        {}

        Point(int X, int Y):
            x(X), y(Y)
        {}

        int x; /**< x coordinate */
        int y; /**< y coordinate */

        /**
         * Check whether the given point is within range of this point.
         */
        bool inRangeOf(const Point &p, int radius) const
        {
            return std::abs(x - p.x) <= radius &&
                   std::abs(y - p.y) <= radius;
        }

        bool operator== (const Point &other) const
        {
            return (x == other.x && y == other.y);
        }

        bool operator!= (const Point &other) const
        {
            return (x != other.x || y != other.y);
        }
};

/**
 * A rectangle. Usually represents a pixel-based zone on a map.
 */
class ManaRectangle
{
    public:
        int x; /**< x coordinate */
        int y; /**< y coordinate */
        int w; /**< width */
        int h; /**< height */

        bool contains(const Point &p) const
        {
            return (p.x >= x && p.x < x + w &&
                    p.y >= y && p.y < y + h);
        }

        bool intersects(const ManaRectangle &r) const
        {
            return x < (r.x + r.w) &&
                   y < (r.y + r.h) &&
                   x + w > r.x &&
                   y + h > r.y;
        }
};

inline std::ostream &operator <<(std::ostream &os, const Point &point)
{
    os << '(' << point.x << ", " << point.y << ')';
    return os;
}

inline std::ostream &operator <<(std::ostream &os, const ManaRectangle &rect)
{
    os << '(' << rect.x << ',' << rect.y
       << ' ' << rect.w << 'x' << rect.h << ')';
    return os;
}

#endif // POINT_H
