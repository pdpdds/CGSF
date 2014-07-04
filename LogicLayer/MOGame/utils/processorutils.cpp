/*
 *  The Mana Server
 *  Copyright (C) 2007-2010  The Mana World Development Team
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

#include "utils/processorutils.h"

bool utils::processor::isLittleEndian;

void utils::processor::init()
{
    utils::processor::isLittleEndian = utils::processor::littleEndianCheck();
}

bool utils::processor::littleEndianCheck()
{
   short int word = 0x0001;     // Store 0x0001 in a 16-bit int.
   char *byte = (char *) &word; // 'byte' points to the first byte in word.
   return(byte[0]);          // byte[0] will be 1 on little-endian processors.
}
