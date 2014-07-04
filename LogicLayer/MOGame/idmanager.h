/*
 *  The Mana Server
 *  Copyright (C) 2013  The Mana Developers
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

#ifndef IDMANAGER_H
#define IDMANAGER_H

#include <unordered_map>

/**
 * A very simplistic ID manager.
 *
 * Does not have any error handling on the premise that other problems will
 * occur before hitting UINT_MAX allocated IDs.
 */
template <typename Value>
class IdManager
{
public:
    IdManager() : mLastId(0) {}
    IdManager(const IdManager&) = delete;

    unsigned allocate(Value *t);
    void free(unsigned id);
    Value *find(unsigned id) const;

private:
    std::unordered_map<unsigned, Value*> mIdMap;
    unsigned mLastId;
};


template <typename Value>
inline unsigned IdManager<Value>::allocate(Value *t)
{
    do {
       ++mLastId;
    } while (mIdMap.find(mLastId) != mIdMap.end());

    mIdMap.emplace(mLastId, t);
    return mLastId;
}

template <typename Value>
inline void IdManager<Value>::free(unsigned id)
{
    mIdMap.erase(id);
}

template <typename Value>
inline Value *IdManager<Value>::find(unsigned id) const
{
    auto it = mIdMap.find(id);
    return it != mIdMap.end() ? it->second : nullptr;
}

#endif // IDMANAGER_H
