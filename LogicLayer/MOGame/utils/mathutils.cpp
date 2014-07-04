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

#include "mathutils.h"

#include <cmath>
#include <stdint.h>
#include <string.h>
#include <float.h>

#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923
#endif

static const int MATH_UTILS_MAX_ANGLE = 360;

static float sinList[MATH_UTILS_MAX_ANGLE];
static float cosList[MATH_UTILS_MAX_ANGLE];
static float tanList[MATH_UTILS_MAX_ANGLE];

namespace utils {
namespace math {

/*
 * A very fast function to calculate the approximate inverse square root of a
 * floating point value. For an explanation of the inverse squareroot function
 * read:
 * http://www.math.purdue.edu/~clomont/Math/Papers/2003/InvSqrt.pdf
 *
 * Unfortunately the original creator of this function seems to be unknown.
 *
 * I wholeheartedly disagree with the use of this function -- silene
 */
float fastInvSqrt(float x)
{
    static_assert(sizeof(float) == 4, "float must be 32 bits");

    float xhalf = 0.5f * x;
    uint32_t i;
    memcpy(&i, &x, 4);
    i = 0x5f375a86 - (i >> 1);
    memcpy(&x, &i, 4);
    x = x * (1.5f-xhalf * x * x);
    return x;
}

void init()
{
    // Constant for calculating an angle in radians out of an angle in degrees
    const float radianAngleRatio = M_PI_2 / 90.0f; // pi/2 / 90[deg]

    for (int i = 0; i < MATH_UTILS_MAX_ANGLE; i++)
    {
        sinList[i] = sin(radianAngleRatio * (float) i);
        cosList[i] = cos(radianAngleRatio * (float) i);

        if (i == 90)
        {
            tanList[i] = FLT_MAX; // approximately infinity
            continue;
        }
        if (i == 270)
        {
            tanList[i] = -FLT_MAX; // approximately infinity
            continue;
        }
        tanList[i] = tan(radianAngleRatio * (float) i);
    }
}

float cachedSin(int angle)
{
    return sinList[angle];
}

float cachedCos(int angle)
{
    return cosList[angle];
}

float cachedTan(int angle)
{
    return tanList[angle];
}

} // namespace math
} // namespace utils
