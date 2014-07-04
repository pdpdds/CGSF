/*
 *  The Mana Server
 *  Copyright (C) 2007-2010  The Mana World Development Team
 *  Copyright (C) 2012  The Mana Developers
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

#include "entity.h"

IdManager<Entity> Entity::mIdManager;

Entity::Entity(EntityType type, MapComposite *map) :
    mId(mIdManager.allocate(this)),
    mMap(map),
    mType(type)
{
    for (int i = 0; i < ComponentTypeCount; ++i)
        mComponents[i] = nullptr;
}

Entity::~Entity()
{
    for (int i = 0; i < ComponentTypeCount; ++i)
        delete mComponents[i];

    mIdManager.free(mId);
}

/**
 * Updates the internal status. By default, calls update on all its components.
 */
void Entity::update()
{
    for (int i = 0; i < ComponentTypeCount; ++i)
        if (mComponents[i])
            mComponents[i]->update(*this);
}
