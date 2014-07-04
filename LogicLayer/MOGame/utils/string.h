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

#ifndef UTILS_STRING_H
#define UTILS_STRING_H

#include <string>
#include <sstream>
#include <map>

namespace utils
{
    /**
     * Returns an upper-cased copy of the string.
     */
    std::string toUpper(std::string);

    /**
     * Returns an lower-cased copy of the string.
     */
    std::string toLower(std::string);

    /**
     * Tells whether the string is a numerical representation.
     */
    bool isNumeric(const std::string &);

    /**
     * Turns a string representing a numerical representation
     * into an integer value.
     */
    int stringToInt(const std::string &);

    /**
     * Compares the two strings case-insensitively.
     *
     * @param a the first string in the comparison
     * @param b the second string in the comparison
     * @return 0 if the strings are equal, positive if the first is greater,
     *           negative if the second is greater
     */
    int compareStrI(const std::string &a, const std::string &b);

    /**
     * Returns the boolean value represented in a string, or default.
     */
    bool stringToBool(const std::string &s, bool defaultValue);

    /**
     * Trims spaces off the end and the beginning of the given string.
     *
     * @param str the string to trim spaces off
     */
    void trim(std::string &str);

    /**
     * Converts the given value to a string using std::stringstream.
     *
     * @param arg the value to convert to a string
     * @return the string representation of arg
     */
    template<typename T> std::string toString(const T &arg)
    {
        std::stringstream ss;
        ss << arg;
        return ss.str();
    }

    /**
     * A case-insensitive name map, mapping instances from a user-specified
     * type by their name.
     */
    template<typename T> class NameMap
    {
    public:
        NameMap()
            : mDefault()
        {}

        void insert(const std::string &name, T value)
        {
            mMap.insert(std::make_pair(toLower(name), value));
        }

        const T &value(const std::string &name) const
        {
            typename Map::const_iterator result = mMap.find(toLower(name));
            return result != mMap.end() ? result->second : mDefault;
        }

        T &operator[](const std::string &name)
        {
            return mMap[toLower(name)];
        }

        bool contains(const std::string &name) const
        {
            return mMap.find(toLower(name)) != mMap.end();
        }

        void clear()
        {
            mMap.clear();
        }

    private:
        typedef std::map<std::string, T> Map;

        Map mMap;
        const T mDefault;
    };

} // namespace utils

#endif // UTILS_STRING_H
