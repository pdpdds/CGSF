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

#include "actorcomponent.h"

#include "map.h"
#include "mapcomposite.h"

#include <cassert>

ActorComponent::ActorComponent(Entity &entity):
    mMoveTime(0),
    mUpdateFlags(0),
    mPublicID(65535),
    mSize(0),
    mWalkMask(0),
    mBlockType(BLOCKTYPE_NONE)
{
    entity.signal_removed.connect(
            sigc::mem_fun(this, &ActorComponent::removed));
    entity.signal_map_changed.connect(
            sigc::mem_fun(this, &ActorComponent::mapChanged));
}

void ActorComponent::removed(Entity *entity)
{
    // Free the map position
    if (MapComposite *mapComposite = entity->getMap())
    {
        Map *map = mapComposite->getMap();
        int tileWidth = map->getTileWidth();
        int tileHeight = map->getTileHeight();
        Point oldP = getPosition();
        map->freeTile(oldP.x / tileWidth, oldP.y / tileHeight, getBlockType());
    }
}

void ActorComponent::setPosition(Entity &entity, const Point &p)
{
    // Update blockmap
    if (MapComposite *mapComposite = entity.getMap())
    {
        Map *map = mapComposite->getMap();
        int tileWidth = map->getTileWidth();
        int tileHeight = map->getTileHeight();
        if ((mPos.x / tileWidth != p.x / tileWidth
            || mPos.y / tileHeight != p.y / tileHeight))
        {
            map->freeTile(mPos.x / tileWidth, mPos.y / tileHeight,
                          getBlockType());
            map->blockTile(p.x / tileWidth, p.y / tileHeight, getBlockType());
        }
    }

    mPos = p;
}

void ActorComponent::mapChanged(Entity *entity)
{
    const Point p = getPosition();

    Map *map = entity->getMap()->getMap();
    int tileWidth = map->getTileWidth();
    int tileHeight = map->getTileHeight();
    map->blockTile(p.x / tileWidth, p.y / tileHeight, getBlockType());
    /* the last line might look illogical because the current position is
     * invalid on the new map, but it is necessary to block the old position
     * because the next call of setPosition() will automatically free the old
     * position. When we don't block the position now the occupation counting
     * will be off.
     */
}
