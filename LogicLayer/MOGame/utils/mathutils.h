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

#ifndef MATHUTILS_H
#define MATHUTILS_H

namespace utils {
namespace math {

/**
 * A very fast function to calculate the approximate inverse square
 * root of a floating point value.
 */
float fastInvSqrt(float x);

/**
 * A helper function that uses the fastInvSqrt for getting the
 * normal squareroot.
 */
inline float fastSqrt(float x)
{
    return x * fastInvSqrt(x);
}

float cachedSin(int angle);
float cachedCos(int angle);
float cachedTan(int angle);

/**
 * Pre-calculates the needed trigomic function values. Should be called before
 * using cachedSin, cachedCos or cachedTan.
 */
void init();

} // namespace math
} // namespace utils

#endif
