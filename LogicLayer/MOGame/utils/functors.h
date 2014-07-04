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

#ifndef FUNCTORS_H
#define FUNCTORS_H

#include <functional>
#include <sstream>
#include <string>

namespace
{

/**
 * Functor to convert a string into another type using
 * std::istringstream.operator>>().
 */
template <typename T>
struct string_to: public std::unary_function<std::string, T>
{
    T
    operator()(const std::string& s) const
    {
        std::istringstream is(s);
        T value;
        is >> value;

        return value;
    }
};

} // anonymous namespace

#endif // FUNCTORS_H
