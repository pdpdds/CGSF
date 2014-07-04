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

#ifndef TOKENDISPENSER_H
#define TOKENDISPENSER_H

#define MAGIC_TOKEN_LENGTH 32

#include <string>

namespace utils
{
    /**
     * \brief Returns a magic_token.
     *
     * The tokens are used for spanning a user's session across multiple
     * servers.
     * NOTE: Uniqueness is not guaranteed, store the account- or characterId
     *       with the token if that is an issue.
     * NOTE: Not passed-by-reference by design.
     * NOTE: Store the token in a variable in this namespace if you want to
     *       avoid 1 copy operation per use.
     */
    std::string getMagicToken();

} // namespace utils

#endif // TOKENDISPENSER_H
