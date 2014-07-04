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

#include "utils/string.h"

#include <cctype>
#include <algorithm>
#include <sstream>

namespace utils {

std::string toUpper(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(), (int(*)(int)) std::toupper);
    return s;
}

std::string toLower(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(), (int(*)(int)) std::tolower);
    return s;
}

bool isNumeric(const std::string &s)
{
    for (unsigned i = 0; i < s.size(); ++i)
    {
        if (!isdigit(s[i]))
        {
            return false;
        }
    }

    return true;
}

int stringToInt(const std::string &s)
{
    int value;
    std::stringstream str(s);

    // put the string into the int
    str >> value;

    return value;
}

int compareStrI(const std::string &a, const std::string &b)
{
    std::string::const_iterator itA = a.begin();
    std::string::const_iterator endA = a.end();
    std::string::const_iterator itB = b.begin();
    std::string::const_iterator endB = b.end();

    for (; itA < endA, itB < endB; ++itA, ++itB)
    {
        int comp = tolower(*itA) - tolower(*itB);
        if (comp)
            return comp;
    }

    // See which one is longer, if either
    if (itA == endA && itB != endB)
        return -1;
    else if (itA != endA && itB == endB)
        return 1;
    else
        return 0;
}

bool stringToBool(const std::string &s, bool defaultValue)
{
    std::string a = toLower(s);
    if (a == "true" || a == "1" || a == "on" || a == "yes" || a == "y")
        return true;
    if (a == "false" || a == "0" || a == "off" || a == "no" || a == "n")
        return false;

    return defaultValue;
}

void trim(std::string &str)
{
    std::string::size_type pos = str.find_last_not_of(" \n\t");
    if (pos != std::string::npos)
    {
        str.erase(pos + 1);
        pos = str.find_first_not_of(" \n\t");
        if (pos != std::string::npos)
            str.erase(0, pos);
    }
    else
    {
        // There is nothing else but whitespace in the string
        str.clear();
    }
}

} // namespace utils
