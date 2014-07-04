/*
 *  The Mana Server
 *  Copyright (C) 2004-2011  The Mana World Development Team
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

#include <algorithm>
#include <queue>
#include <cassert>
#include <cstring>
#include <limits.h>

#include "map.h"

#include "common/defines.h"

/**
 * Stores information used during path finding for each tile of a map.
 */
class PathInfo
{
    public:
        PathInfo()
            : Gcost(0)
            , Hcost(0)
            , whichList(0)
            , parentX(0)
            , parentY(0)
        {}

        int Gcost;              /**< Cost from start to this location */
        int Hcost;              /**< Estimated cost to goal */
        unsigned whichList;     /**< No list, open list or closed list */
        int parentX;            /**< X coordinate of parent tile */
        int parentY;            /**< Y coordinate of parent tile */
};

/**
 * A helper class for finding a path on a map, functor style.
 */
class FindPath
{
    public:
        FindPath() :
            mWidth(0),
            mOnClosedList(1),
            mOnOpenList(2)
        {}

        Path operator() (int startX, int startY,
                         int destX, int destY,
                         unsigned char walkmask, int maxCost,
                         const Map *map);

    private:
        PathInfo *getInfo(int x, int y)
        { return &mPathInfos.at(x + y * mWidth); }

        void prepare(const Map *map);

        int mWidth;
        std::vector<PathInfo> mPathInfos;
        unsigned mOnClosedList, mOnOpenList;
};

static FindPath findPath;


/**
 * A location on a tile map. Used for pathfinding, open list.
 */
class Location
{
    public:
        Location(int x, int y, int Fcost):
            x(x), y(y), Fcost(Fcost)
        {}

        /**
         * Comparison operator.
         */
        bool operator< (const Location &other) const
        { return Fcost > other.Fcost; }

        int x, y;
        int Fcost;              /**< Estimation of total path cost */
};

Map::Map(int width, int height, int tileWidth, int tileHeight):
    mWidth(width), mHeight(height),
    mTileWidth(tileWidth), mTileHeight(tileHeight),
    mMetaTiles(width * height)
{
}

Map::~Map()
{
    for (std::vector<MapObject*>::iterator it = mMapObjects.begin();
         it != mMapObjects.end(); ++it)
    {
        delete *it;
    }
}

void Map::setSize(int width, int height)
{
    mWidth = width;
    mHeight = height;

    mMetaTiles.resize(width * height);
}

const std::string &Map::getProperty(const std::string &key) const
{
    static std::string empty;
    std::map<std::string, std::string>::const_iterator i;
    i = mProperties.find(key);
    if (i == mProperties.end())
        return empty;
    return i->second;
}

void Map::blockTile(int x, int y, BlockType type)
{
    if (type == BLOCKTYPE_NONE || !contains(x, y))
        return;

    MetaTile &metaTile = mMetaTiles[x + y * mWidth];

    if (metaTile.occupation[type] < UINT_MAX &&
        (++metaTile.occupation[type]) > 0)
    {
        switch (type)
        {
            case BLOCKTYPE_WALL:
                metaTile.blockmask |= BLOCKMASK_WALL;
                break;
            case BLOCKTYPE_CHARACTER:
                metaTile.blockmask |= BLOCKMASK_CHARACTER;
                break;
            case BLOCKTYPE_MONSTER:
                metaTile.blockmask |= BLOCKMASK_MONSTER;
                break;
            default:
                // Nothing to do.
                break;
        }
    }
}

void Map::freeTile(int x, int y, BlockType type)
{
    if (type == BLOCKTYPE_NONE || !contains(x, y))
        return;

    MetaTile &metaTile = mMetaTiles[x + y * mWidth];
    assert(metaTile.occupation[type] > 0);

    if (!(--metaTile.occupation[type]))
    {
        switch (type)
        {
            case BLOCKTYPE_WALL:
                metaTile.blockmask &= (BLOCKMASK_WALL ^ 0xff);
                break;
            case BLOCKTYPE_CHARACTER:
                metaTile.blockmask &= (BLOCKMASK_CHARACTER ^ 0xff);
                break;
            case BLOCKTYPE_MONSTER:
                metaTile.blockmask &= (BLOCKMASK_MONSTER ^ 0xff);
                break;
            default:
                // nothing
                break;
        }
    }
}

bool Map::getWalk(int x, int y, char walkmask) const
{
    // You can't walk outside of the map
    if (!contains(x, y))
        return false;

    // Check if the tile is walkable
    return !(mMetaTiles[x + y * mWidth].blockmask & walkmask);
}

Path Map::findPath(int startX, int startY,
                   int destX, int destY,
                   unsigned char walkmask, int maxCost) const
{
    return ::findPath(startX, startY,
                      destX, destY,
                      walkmask, maxCost,
                      this);
}

Path FindPath::operator() (int startX, int startY,
                           int destX, int destY,
                           unsigned char walkmask, int maxCost,
                           const Map *map)
{
    // Basic cost for moving from one tile to another.
    static int const basicCost = 100;

    // Path to be built up (empty by default)
    Path path;

    // Return when destination not walkable
    if (!map->getWalk(destX, destY, walkmask))
        return path;

    prepare(map);

    // Declare open list, a list with open tiles sorted on F cost
    std::priority_queue<Location> openList;

    // Reset starting tile's G cost to 0
    PathInfo *startTile = getInfo(startX, startY);
    startTile->Gcost = 0;

    // Add the start point to the open list (F cost irrelevant here)
    openList.push(Location(startX, startY, 0));

    bool foundPath = false;

    // Keep trying new open tiles until no more tiles to try or target found
    while (!openList.empty() && !foundPath)
    {
        // Take the location with the lowest F cost from the open list, and
        // add it to the closed list.
        Location curr = openList.top();
        openList.pop();
        PathInfo *currInfo = getInfo(curr.x, curr.y);

        // If the tile is already on the closed list, this means it has already
        // been processed with a shorter path to the start point (lower G cost)
        if (currInfo->whichList == mOnClosedList)
            continue;

        // Put the current tile on the closed list
        currInfo->whichList = mOnClosedList;

        // Check the adjacent tiles
        for (int dy = -1; dy <= 1; dy++)
        {
            for (int dx = -1; dx <= 1; dx++)
            {
                // Calculate location of tile to check
                int x = curr.x + dx;
                int y = curr.y + dy;

                // Skip if if we're checking the same tile we're leaving from,
                // or if the new location falls outside of the map boundaries
                if ((dx == 0 && dy == 0) || !map->contains(x, y))
                    continue;

                PathInfo *newTile = getInfo(x, y);

                // Skip if the tile is on the closed list or is not walkable
                if (newTile->whichList == mOnClosedList
                        || !map->getWalk(x, y, walkmask))
                    continue;

                // When taking a diagonal step, verify that we can skip the
                // corner.
                if (dx != 0 && dy != 0)
                {
                    if (!map->getWalk(curr.x, curr.y + dy, walkmask)
                            || !map->getWalk(curr.x + dx, curr.y, walkmask))
                        continue;
                }

                // Calculate G cost for this route, ~sqrt(2) for moving diagonal
                int Gcost = currInfo->Gcost +
                    (dx == 0 || dy == 0 ? basicCost : basicCost * 362 / 256);

                /* Demote an arbitrary direction to speed pathfinding by
                   adding a defect (TODO: change depending on the desired
                   visual effect, e.g. a cross-product defect toward
                   destination).
                   Important: as long as the total defect along any path is
                   less than the basicCost, the pathfinder will still find one
                   of the shortest paths! */
                if (dx == 0 || dy == 0)
                {
                    // Demote horizontal and vertical directions, so that two
                    // consecutive directions cannot have the same Fcost.
                    ++Gcost;
                }

                // Skip if Gcost becomes too much
                // Warning: probably not entirely accurate
                if (Gcost > maxCost * basicCost)
                    continue;

                if (newTile->whichList != mOnOpenList)
                {
                    // Found a new tile (not on open nor on closed list)

                    /* Update Hcost of the new tile. The pathfinder does not
                       work reliably if the heuristic cost is higher than the
                       real cost. In particular, using Manhattan distance is
                       forbidden here. */
                    int dx = std::abs(x - destX), dy = std::abs(y - destY);
                    newTile->Hcost = std::abs(dx - dy) * basicCost +
                        std::min(dx, dy) * (basicCost * 362 / 256);

                    // Set the current tile as the parent of the new tile
                    newTile->parentX = curr.x;
                    newTile->parentY = curr.y;

                    // Update Gcost of new tile
                    newTile->Gcost = Gcost;

                    if (x != destX || y != destY)
                    {
                        // Add this tile to the open list
                        newTile->whichList = mOnOpenList;
                        openList.push(Location(x, y, Gcost + newTile->Hcost));
                    }
                    else
                    {
                        // Target location was found
                        foundPath = true;
                    }
                }
                else if (Gcost < newTile->Gcost)
                {
                    // Found a shorter route.
                    // Update Gcost of the new tile
                    newTile->Gcost = Gcost;

                    // Set the current tile as the parent of the new tile
                    newTile->parentX = curr.x;
                    newTile->parentY = curr.y;

                    // Add this tile to the open list (it's already
                    // there, but this instance has a lower F score)
                    openList.push(Location(x, y, Gcost + newTile->Hcost));
                }
            }
        }
    }

    // If a path has been found, iterate backwards using the parent locations
    // to extract it.
    if (foundPath)
    {
        int pathX = destX;
        int pathY = destY;

        while (pathX != startX || pathY != startY)
        {
            // Add the new path node to the start of the path list
            path.push_front(Point(pathX, pathY));

            // Find out the next parent
            PathInfo *tile = getInfo(pathX, pathY);
            pathX = tile->parentX;
            pathY = tile->parentY;
        }
    }

    return path;
}

void FindPath::prepare(const Map *map)
{
    // Two new values to indicate whether a tile is on the open or closed list,
    // this way we don't have to clear all the values between each pathfinding.
    if (mOnOpenList < UINT_MAX - 2)
    {
        mOnClosedList += 2;
        mOnOpenList += 2;
    }
    else
    {
        // Reset closed and open list IDs and clear the whichList values
        mOnClosedList = 1;
        mOnOpenList = 2;
        for (unsigned i = 0, end = mPathInfos.size(); i < end; ++i)
            mPathInfos[i].whichList = 0;
    }

    // Make sure we have enough room to cover this map with path information
    const unsigned size = map->getWidth() * map->getHeight();
    if (mPathInfos.size() < size)
        mPathInfos.resize(size);

    mWidth = map->getWidth();
}
