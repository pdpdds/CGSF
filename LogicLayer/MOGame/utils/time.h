/*
 *  The Mana Server
 *  Copyright (C) 2011  The Mana Development Team
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

#ifndef TIME_H
#define TIME_H

#include <ctime>
#include <iomanip>
#include <string>

namespace utils {

/**
  * Gets the current time.
  *
  * @return the current time as string.
  */
static std::string getCurrentTime()
{
    time_t now;
    tm local;

    // Get current time_t value
    time(&now);

    // Convert time_t to tm struct to break the time into individual
    // constituents.
    local = *(localtime(&now));

    // Stringify the time, the format is: hh:mm:ss
    using namespace std;
    ostringstream os;
    os << setw(2) << setfill('0') << local.tm_hour
       << ":" << setw(2) << setfill('0') << local.tm_min
       << ":" << setw(2) << setfill('0') << local.tm_sec;

    return os.str();
}

/**
  * Gets the current date.
  *
  * @return the current date as string.
  */
static std::string getCurrentDate()
{
    time_t now;
    tm local;

    // Get current time_t value
    time(&now);

    // Convert time_t to tm struct to break the time into individual
    // constituents.
    local = *(localtime(&now));

    // Stringify the time, the format is: yyyy-mm-dd
    using namespace std;
    ostringstream os;
    os << setw(4) << setfill('0') << (local.tm_year + 1900)
       << "-" << setw(2) << setfill('0') << local.tm_mon + 1
       << "-" << setw(2) << setfill('0') << local.tm_mday;

    return os.str();
}

} // namespace utils

#endif // TIME_H
