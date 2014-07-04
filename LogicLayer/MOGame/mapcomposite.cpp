/*
 *  The Mana Server
 *  Copyright (C) 2006-2010  The Mana World Development Team
 *  Copyright (C) 2010-2012  The Mana Development Team
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
#include <cassert>

#include "accountconnection.h"
#include "common/configuration.h"
#include "common/resourcemanager.h"
#include "charactercomponent.h"
#include "mapcomposite.h"
#include "map.h"
#include "mapmanager.h"
#include "mapreader.h"
#include "monstermanager.h"
#include "spawnareacomponent.h"
#include "triggerareacomponent.h"
#include "scripting/script.h"
#include "scripting/scriptmanager.h"
#include "utils/logger.h"
#include "utils/point.h"
#include <algorithm>
/******************************************************************************
 * ObjectBucket
 *****************************************************************************/

/**
 * Pool of public IDs for MovingObjects on a map. By maintaining public ID
 * availability using bits, it can locate an available public ID fast while
 * using minimal memory access.
 */
struct ObjectBucket
{
    static int const int_bitsize = sizeof(unsigned) * 8;
    unsigned bitmap[256 / int_bitsize]; /**< Bitmap of free locations. */
    short free;                         /**< Number of empty places. */
    short next_object;                  /**< Next object to look at. */
    Entity *objects[256];

    ObjectBucket();
    int allocate();
    void deallocate(int);
    bool isAllocated(int) const;
};

ObjectBucket::ObjectBucket()
  : free(256), next_object(0)
{
    for (unsigned i = 0; i < 256 / int_bitsize; ++i)
    {
        // An occupied ID is represented by zero in the bitmap.
        bitmap[i] = ~0u;
    }
}

int ObjectBucket::allocate()
{
    // Any free ID in the bucket?
    if (!free)
    {
        LOG_INFO("No free id in bucket");
        return -1;
    }

    int freeBucket = -1;
    // See if the the next_object bucket is free
    if (bitmap[next_object] != 0)
    {
        freeBucket = next_object;
    }
    else
    {
        /* next_object was not free. Check the whole bucket until one ID is found,
           starting from the IDs around next_object. */
        for (unsigned i = 0; i < 256 / int_bitsize; ++i)
        {
            // Check to see if this subbucket is free
            if (bitmap[i] != 0)
            {
                freeBucket = i;
                break;
            }
        }
    }

    assert(freeBucket >= 0);

    // One of them is free. Find it by looking bit-by-bit.
    int b = bitmap[freeBucket];
    int j = 0;
    while (!(b & 1))
    {
        b >>= 1;
        ++j;
    }
    // Flip that bit to on, and return the value
    bitmap[freeBucket] &= ~(1 << j);
    j += freeBucket * int_bitsize;
    next_object = freeBucket;
    --free;
    return j;
}

void ObjectBucket::deallocate(int i)
{
    assert(!(bitmap[i / int_bitsize] & (1 << (i % int_bitsize))));
    bitmap[i / int_bitsize] |= 1 << (i % int_bitsize);
    ++free;
}

bool ObjectBucket::isAllocated(int i) const
{
    return !(bitmap[i / int_bitsize] & (1 << (i % int_bitsize)));
}


/******************************************************************************
 * MapZone
 *****************************************************************************/

/* TODO: Implement overlapping map zones instead of strict partitioning.
   Purpose: to decrease the number of zone changes, as overlapping allows for
   hysteresis effect and prevents an actor from changing zone each server
   tick. It requires storing the zone in the actor since it will not be
   uniquely defined any longer. */

/* Pixel-based width and height of the squares used in partitioning the map.
   Squares should be big enough so that an actor cannot cross several ones
   in one world tick.
   TODO: Tune for decreasing server load. The higher the value, the closer we
   regress to quadratic behavior; the lower the value, the more we waste time
   in dealing with zone changes. */
static int const zoneDiam = 256;

/**
 * Part of a map.
 */
struct MapZone
{
    unsigned short nbCharacters, nbMovingObjects;
    /**
     * Objects present in this zone.
     * Characters are stored first, then the remaining MovingObjects, then the
     * remaining Objects.
     */
    std::vector< Entity * > objects;

    /**
     * Destinations of the objects that left this zone.
     * This is necessary in order to have an accurate iterator around moving
     * objects.
     */
    MapRegion destinations;

    MapZone(): nbCharacters(0), nbMovingObjects(0) {}
    void insert(Entity *);
    void remove(Entity *);
};

void MapZone::insert(Entity *obj)
{
    int type = obj->getType();
    switch (type)
    {
        case OBJECT_CHARACTER:
        {
            if (nbCharacters != nbMovingObjects)
            {
                if (nbMovingObjects != objects.size())
                {
                    objects.push_back(objects[nbMovingObjects]);
                    objects[nbMovingObjects] = objects[nbCharacters];
                }
                else
                {
                    objects.push_back(objects[nbCharacters]);
                }
                objects[nbCharacters] = obj;
                ++nbCharacters;
                ++nbMovingObjects;
                break;
            }
            ++nbCharacters;
        } // no break!
        case OBJECT_MONSTER:
        case OBJECT_NPC:
        {
            if (nbMovingObjects != objects.size())
            {
                objects.push_back(objects[nbMovingObjects]);
                objects[nbMovingObjects] = obj;
                ++nbMovingObjects;
                break;
            }
            ++nbMovingObjects;
        } // no break!
        default:
        {
            objects.push_back(obj);
            break;
        }
    }
}

void MapZone::remove(Entity *obj)
{
    std::vector< Entity * >::iterator i_beg = objects.begin(), i, i_end;
    int type = obj->getType();
    switch (type)
    {
        case OBJECT_CHARACTER:
        {
            i = i_beg;
            i_end = objects.begin() + nbCharacters;
        } break;
        case OBJECT_MONSTER:
        case OBJECT_NPC:
        {
            i = objects.begin() + nbCharacters;
            i_end = objects.begin() + nbMovingObjects;
        } break;
        default:
        {
            i = objects.begin() + nbMovingObjects;
            i_end = objects.end();
            break;
        }
    }
    i = std::find(i, i_end, obj);
    assert(i != i_end);
    unsigned pos = i - i_beg;
    if (pos < nbCharacters)
    {
        objects[pos] = objects[nbCharacters - 1];
        pos = nbCharacters - 1;
        --nbCharacters;
    }
    if (pos < nbMovingObjects)
    {
        objects[pos] = objects[nbMovingObjects - 1];
        pos = nbMovingObjects - 1;
        --nbMovingObjects;
    }
    objects[pos] = objects[objects.size() - 1];
    objects.pop_back();
}

/******************************************************************************
 * MapContent
 *****************************************************************************/

/**
 * Entities on a map.
 */
struct MapContent
{
    MapContent(Map *);
    ~MapContent();

    /**
     * Allocates a unique ID for an actor on this map.
     */
    bool allocate(Entity *);

    /**
     * Deallocates an ID.
     */
    void deallocate(Entity *);

    Entity *findEntityById(int publicId) const;

    /**
     * Fills a region of zones within the range of a point.
     */
    void fillRegion(MapRegion &, const Point &, int) const;

    /**
     * Fills a region of zones inside a rectangle.
     */
	void fillRegion(MapRegion &, const ManaRectangle &) const;

    /**
     * Gets zone at given position.
     */
    MapZone &getZone(const Point &pos) const;

    /**
     * Entities (items, characters, monsters, etc) located on the map.
     */
    std::vector< Entity * > entities;

    /**
     * Buckets of MovingObjects located on the map, referenced by ID.
     */
    ObjectBucket *buckets[256];

    int last_bucket; /**< Last bucket acted upon. */

    /**
     * Partition of the Objects, depending on their position on the map.
     */
    MapZone *zones;

    unsigned short mapWidth;  /**< Width with respect to zones. */
    unsigned short mapHeight; /**< Height with respect to zones. */
};

MapContent::MapContent(Map *map)
  : last_bucket(0), zones(nullptr)
{
    buckets[0] = new ObjectBucket;
    buckets[0]->allocate(); // Skip ID 0
    for (int i = 1; i < 256; ++i)
    {
        buckets[i] = nullptr;
    }
    mapWidth = (map->getWidth() * map->getTileWidth() + zoneDiam - 1)
               / zoneDiam;
    mapHeight = (map->getHeight() * map->getTileHeight() + zoneDiam - 1)
                / zoneDiam;
    zones = new MapZone[mapWidth * mapHeight];
}

MapContent::~MapContent()
{
    for (int i = 0; i < 256; ++i)
    {
        delete buckets[i];
    }
    delete[] zones;
}

bool MapContent::allocate(Entity *obj)
{
    // First, try allocating from the last used bucket.
    ObjectBucket *b = buckets[last_bucket];

    auto *actorComponent = obj->getComponent<ActorComponent>();

    int i = b->allocate();
    if (i >= 0)
    {
        b->objects[i] = obj;
        actorComponent->setPublicID(last_bucket * 256 + i);
        return true;
    }

    /* If the last used bucket is already full, scan all the buckets for an
       empty place. If none is available, create a new bucket. */
    for (i = 0; i < 256; ++i)
    {
        b = buckets[i];
        if (!b)
        {
            /* Buckets are created in order. If there is nothing at position i,
               there will not be anything in the next positions. So create a
               new bucket. */
            b = new ObjectBucket;
            buckets[i] = b;
            LOG_DEBUG("New bucket created");
        }
        int j = b->allocate();
        if (j >= 0)
        {
            last_bucket = i;
            b->objects[j] = obj;
            actorComponent->setPublicID(last_bucket * 256 + j);
            return true;
        }
    }

    // All the IDs are currently used, fail.
    LOG_ERROR("unable to allocate id");
    return false;
}

void MapContent::deallocate(Entity *obj)
{
    unsigned short id = obj->getComponent<ActorComponent>()->getPublicID();
    buckets[id / 256]->deallocate(id % 256);
}

/**
 * Returns the entity matching \a publicId, or null if no such entity exists.
 */
Entity *MapContent::findEntityById(int publicId) const
{
    if (ObjectBucket *b = buckets[publicId / 256]) {
        const int bucketIndex = publicId % 256;
        if (b->isAllocated(bucketIndex))
            return b->objects[bucketIndex];
    }
    return 0;
}

static void addZone(MapRegion &r, unsigned z)
{
    MapRegion::iterator i_end = r.end(),
                        i = std::lower_bound(r.begin(), i_end, z);
    if (i == i_end || *i != z)
    {
        r.insert(i, z);
    }
}

void MapContent::fillRegion(MapRegion &r, const Point &p, int radius) const
{
    int ax = p.x > radius ? (p.x - radius) / zoneDiam : 0,
        ay = p.y > radius ? (p.y - radius) / zoneDiam : 0,
        bx = std::min((p.x + radius) / zoneDiam, mapWidth - 1),
        by = std::min((p.y + radius) / zoneDiam, mapHeight - 1);
    for (int y = ay; y <= by; ++y)
    {
        for (int x = ax; x <= bx; ++x)
        {
            addZone(r, x + y * mapWidth);
        }
    }
}

void MapContent::fillRegion(MapRegion &r, const ManaRectangle &p) const
{
    int ax = p.x / zoneDiam,
        ay = p.y / zoneDiam,
        bx = std::min((p.x + p.w) / zoneDiam, mapWidth - 1),
        by = std::min((p.y + p.h) / zoneDiam, mapHeight - 1);
    for (int y = ay; y <= by; ++y)
    {
        for (int x = ax; x <= bx; ++x)
        {
            addZone(r, x + y * mapWidth);
        }
    }
}

MapZone& MapContent::getZone(const Point &pos) const
{
    return zones[(pos.x / zoneDiam) + (pos.y / zoneDiam) * mapWidth];
}


/******************************************************************************
 * ZoneIterator
 *****************************************************************************/

ZoneIterator::ZoneIterator(const MapRegion &r, const MapContent *m)
  : region(r), pos(0), map(m)
{
    current = &map->zones[r.empty() ? 0 : r[0]];
}

void ZoneIterator::operator++()
{
    current = nullptr;
    if (!region.empty())
    {
        if (++pos != region.size())
        {
            current = &map->zones[region[pos]];
        }
    }
    else
    {
        if (++pos != (unsigned)map->mapWidth * map->mapHeight)
        {
            current = &map->zones[pos];
        }
    }
}

CharacterIterator::CharacterIterator(const ZoneIterator &it)
  : iterator(it), pos(0)
{
    while (iterator && (*iterator)->nbCharacters == 0) ++iterator;
    if (iterator)
    {
        current = (*iterator)->objects[pos];
    }
}

void CharacterIterator::operator++()
{
    if (++pos == (*iterator)->nbCharacters)
    {
        do ++iterator; while (iterator && (*iterator)->nbCharacters == 0);
        pos = 0;
    }
    if (iterator)
    {
        current = (*iterator)->objects[pos];
    }
}

BeingIterator::BeingIterator(const ZoneIterator &it)
  : iterator(it), pos(0)
{
    while (iterator && (*iterator)->nbMovingObjects == 0) ++iterator;
    if (iterator)
    {
        current = (*iterator)->objects[pos];
    }
}

void BeingIterator::operator++()
{
    if (++pos == (*iterator)->nbMovingObjects)
    {
        do ++iterator; while (iterator && (*iterator)->nbMovingObjects == 0);
        pos = 0;
    }
    if (iterator)
    {
        current = (*iterator)->objects[pos];
    }
}

FixedActorIterator::FixedActorIterator(const ZoneIterator &it)
  : iterator(it), pos(0)
{
    while (iterator && (*iterator)->nbMovingObjects == (*iterator)->objects.size()) ++iterator;
    if (iterator)
    {
        pos = (*iterator)->nbMovingObjects;
        current = (*iterator)->objects[pos];
    }
}

void FixedActorIterator::operator++()
{
    if (++pos == (*iterator)->objects.size())
    {
        do ++iterator; while (iterator && (*iterator)->nbMovingObjects == (*iterator)->objects.size());
        if (iterator)
        {
            pos = (*iterator)->nbMovingObjects;
        }
    }
    if (iterator)
    {
        current = (*iterator)->objects[pos];
    }
}

ActorIterator::ActorIterator(const ZoneIterator &it)
  : iterator(it), pos(0)
{
    while (iterator && (*iterator)->objects.empty()) ++iterator;
    if (iterator)
    {
        current = (*iterator)->objects[pos];
    }
}

void ActorIterator::operator++()
{
    if (++pos == (*iterator)->objects.size())
    {
        do ++iterator; while (iterator && (*iterator)->objects.empty());
        pos = 0;
    }
    if (iterator)
    {
        current = (*iterator)->objects[pos];
    }
}


/******************************************************************************
 * MapComposite
 *****************************************************************************/

Script::Ref MapComposite::mInitializeCallback;
Script::Ref MapComposite::mUpdateCallback;

MapComposite::MapComposite(int id, const std::string &name):
    mActive(false),
    mMap(0),
    mContent(0),
    mName(name),
    mID(id),
    mPvPRules(PVP_NONE)
{
}

MapComposite::~MapComposite()
{
    delete mMap;
    delete mContent;
}

bool MapComposite::readMap()
{
    std::string file = "maps/" + mName + ".tmx";

    mMap = MapReader::readMap(file);
    return mMap;
}

bool MapComposite::activate()
{
    assert(!isActive());

    if (!mMap)
        return false;

    initializeContent();

    std::string sPvP = mMap->getProperty("pvp");
    if (sPvP.empty())
        sPvP = Configuration::getValue("game_defaultPvp", std::string());

    if (sPvP == "free")
        mPvPRules = PVP_FREE;
    else
        mPvPRules = PVP_NONE;

    mActive = true;

    if (!mInitializeCallback.isValid())
    {
        LOG_WARN("No callback for map initialization found");
    }
    else
    {
        Script *s = ScriptManager::currentState();
        s->prepare(mInitializeCallback);
        s->execute(this);
    }

    return true;
}

ZoneIterator MapComposite::getAroundPointIterator(const Point &p, int radius) const
{
    MapRegion r;
    mContent->fillRegion(r, p, radius);
    return ZoneIterator(r, mContent);
}

ZoneIterator MapComposite::getAroundActorIterator(Entity *obj, int radius) const
{
    MapRegion r;
    mContent->fillRegion(r, obj->getComponent<ActorComponent>()->getPosition(),
                         radius);
    return ZoneIterator(r, mContent);
}

ZoneIterator MapComposite::getInsideRectangleIterator(const ManaRectangle &p) const
{
    MapRegion r;
    mContent->fillRegion(r, p);
    return ZoneIterator(r, mContent);
}

ZoneIterator MapComposite::getAroundBeingIterator(Entity *obj, int radius) const
{
    MapRegion r1;
    mContent->fillRegion(r1,
                         obj->getComponent<BeingComponent>()->getOldPosition(),
                         radius);
    MapRegion r2 = r1;
    for (MapRegion::iterator i = r1.begin(), i_end = r1.end(); i != i_end; ++i)
    {
        /* Fills region with destinations taken around the old position.
           This is necessary to detect two moving objects changing zones at the
           same time and at the border, and going in opposite directions (or
           more simply to detect teleportations, if any). */
        MapRegion &r4 = mContent->zones[*i].destinations;
        if (!r4.empty())
        {
            MapRegion r3;
            r3.reserve(r2.size() + r4.size());
            std::set_union(r2.begin(), r2.end(), r4.begin(), r4.end(),
                           std::back_insert_iterator< MapRegion >(r3));
            r2.swap(r3);
        }
    }
    mContent->fillRegion(r2,
                         obj->getComponent<ActorComponent>()->getPosition(),
                         radius);
    return ZoneIterator(r2, mContent);
}

bool MapComposite::insert(Entity *ptr)
{
    if (ptr->isVisible())
    {
        if (ptr->canMove() && !mContent->allocate(ptr))
            return false;

        const Point &point =
                ptr->getComponent<ActorComponent>()->getPosition();
        mContent->getZone(point).insert(ptr);
    }

    ptr->setMap(this);
    mContent->entities.push_back(ptr);
    return true;
}

void MapComposite::remove(Entity *ptr)
{
    for (std::vector<Entity*>::iterator i = mContent->entities.begin(),
         i_end = mContent->entities.end(); i != i_end; ++i)
    {
        if (*i == ptr)
        {
            i = mContent->entities.erase(i);
        }
    }

    if (ptr->isVisible())
    {
        const Point &point =
                ptr->getComponent<ActorComponent>()->getPosition();
        mContent->getZone(point).remove(ptr);

        if (ptr->canMove())
        {
            mContent->deallocate(ptr);
        }
    }
}

Entity *MapComposite::findEntityById(int publicId) const
{
    return mContent->findEntityById(publicId);
}

void MapComposite::update()
{
    // Update object status
    const std::vector< Entity * > &entities = getEverything();
    for (std::vector< Entity * >::const_iterator it = entities.begin(),
         it_end = entities.end(); it != it_end; ++it)
    {
        (*it)->update();
    }

    if (mUpdateCallback.isValid())
    {
        Script *s = ScriptManager::currentState();
        s->prepare(mUpdateCallback);
        s->push(mID);
        s->execute(this);
    }

    // Move objects around and update zones.
    for (BeingIterator it(getWholeMapIterator()); it; ++it)
    {
        (*it)->getComponent<BeingComponent>()->move(**it);
    }

    for (int i = 0; i < mContent->mapHeight * mContent->mapWidth; ++i)
    {
        mContent->zones[i].destinations.clear();
    }

    // Cannot use a WholeMap iterator as objects will change zones under its feet.
    for (std::vector< Entity * >::iterator i = mContent->entities.begin(),
         i_end = mContent->entities.end(); i != i_end; ++i)
    {
        if (!(*i)->canMove())
            continue;

        const Point &pos1 =
                (*i)->getComponent<BeingComponent>()->getOldPosition();
        const Point &pos2 =
                (*i)->getComponent<ActorComponent>()->getPosition();

        MapZone &src = mContent->getZone(pos1),
                &dst = mContent->getZone(pos2);
        if (&src != &dst)
        {
            addZone(src.destinations, &dst - mContent->zones);
            src.remove(*i);
            dst.insert(*i);
        }
    }
}

const std::vector< Entity * > &MapComposite::getEverything() const
{
    return mContent->entities;
}


std::string MapComposite::getVariable(const std::string &key) const
{
    std::map<std::string, std::string>::const_iterator i = mScriptVariables.find(key);
    if (i != mScriptVariables.end())
        return i->second;
    else
        return std::string();
}

void MapComposite::setVariable(const std::string &key, const std::string &value)
{
    // check if the value actually changed
    std::map<std::string, std::string>::iterator i = mScriptVariables.find(key);
    if (i == mScriptVariables.end() || i->second != value)
    {
        // changed value or unknown variable
        mScriptVariables[key] = value;
        callMapVariableCallback(key, value);
        // update accountserver
        accountHandler->updateMapVar(this, key, value);
    }
}

static void callVariableCallback(Script::Ref &function, const std::string &key,
                                 const std::string &value, MapComposite *map)
{
    if (function.isValid())
    {
        Script *s = ScriptManager::currentState();
        s->prepare(function);
        s->push(key);
        s->push(value);
        s->execute(map);
    }
}

void MapComposite::callMapVariableCallback(const std::string &key,
                                           const std::string &value)
{
    std::map<const std::string, Script::Ref>::iterator it =
            mMapVariableCallbacks.find(key);
    if (it == mMapVariableCallbacks.end())
        return;
    callVariableCallback(it->second, key, value, this);
}

void MapComposite::callWorldVariableCallback(const std::string &key,
                                             const std::string &value)
{
    std::map<const std::string, Script::Ref>::iterator it =
            mWorldVariableCallbacks.find(key);
    if (it == mWorldVariableCallbacks.end())
        return;
    callVariableCallback(it->second, key, value, this);
}

/**
 * Finds a map object by its name and type.
 * Name and type are case insensitive.
 */
const MapObject *MapComposite::findMapObject(const std::string &name,
                                             const std::string &type) const
{
    const std::vector<MapObject *> &destObjects = mMap->getObjects();
    std::vector<MapObject *>::const_iterator it, it_end;
    for (it = destObjects.begin(), it_end = destObjects.end();
         it != it_end; ++it)
    {
        const MapObject *obj = *it;
        if (utils::compareStrI(obj->getType(), type) == 0 &&
            utils::compareStrI(obj->getName(), name) == 0)
        {
            return obj;
        }
    }
    return 0; // nothing found
}

/**
 * Initializes the map content. This creates the warps, spawn areas, npcs and
 * other scripts.
 */
void MapComposite::initializeContent()
{
    mContent = new MapContent(mMap);

    const std::vector<MapObject *> &objects = mMap->getObjects();

    for (size_t i = 0; i < objects.size(); ++i)
    {
        const MapObject *object = objects.at(i);
        const std::string &type = object->getType();

        if (utils::compareStrI(type, "WARP") == 0)
        {
            const std::string destMapName = object->getProperty("DEST_MAP");
			const ManaRectangle &sourceBounds = object->getBounds();
            MapComposite *destMap = MapManager::getMap(destMapName);

            // check destination map
            if (!destMap)
            {
                if (destMapName.empty())
                {
                    // this must be a one way warp target
                    continue;
                }

                LOG_ERROR("Warp \"" << object->getName() << "\" targets missing map \""
                          << destMapName << "\" in " << getName());
                continue;
            }


            TriggerAction* action;
            if (object->hasProperty("DEST_NAME"))
            {
                // warp to an object
                // get destination object name
                const std::string destMapObjectName = object->getProperty("DEST_NAME");
                // get target object and validate it
                const MapObject *destination = destMap->findMapObject(destMapObjectName, "WARP");
                if (!destination)
                {
                    LOG_ERROR("Warp \"" << object->getName() << "\" from map " << getName()
                              << " targets missing warp \"" << destMapObjectName << "\" "
                              << " on map " << destMap->getName());
                    continue;
                }

				const ManaRectangle &destinationBounds = destination->getBounds();

                const std::string &exit = destination->getProperty("EXIT_DIRECTION");

                if (exit.empty()) {
                    // old style WARP, warp to center of that object
                    int destX = destinationBounds.x + destinationBounds.w / 2;
                    int destY = destinationBounds.y + destinationBounds.h / 2;
                    action = new WarpAction(destMap, Point(destX, destY));
                }
                else
                {
                    // newer and cooler warp

                    AutowarpAction::ExitDirection exitDir;

                    // find the exit direction
                    if (utils::compareStrI(exit, "NORTH") == 0)
                    {
                        exitDir = AutowarpAction::ExitNorth;
                    }
                    else if (utils::compareStrI(exit, "EAST") == 0)
                    {
                        exitDir = AutowarpAction::ExitEast;
                    }
                    else if (utils::compareStrI(exit, "SOUTH") == 0)
                    {
                        exitDir = AutowarpAction::ExitSouth;
                    }
                    else if (utils::compareStrI(exit, "WEST") == 0)
                    {
                        exitDir = AutowarpAction::ExitWest;
                    }
                    else
                    {
                        // invalid or missing exit direction
                        if (exit.empty())
                        {
                            LOG_ERROR("Warp target \"" << destMapObjectName << "\" on map "
                                    << destMap->getName()
                                    << " is missing exit direction!");
                        }
                        else
                        {
                            LOG_ERROR("Warp target \"" << destMapObjectName << "\" on map "
                                    << destMap->getName()
                                    << " has an invalid exit direction \""
                                    << exit
                                    << "\"!");
                        }
                        continue;
                    }

                    action = new AutowarpAction(destMap, sourceBounds,
                                                destinationBounds, exitDir);
                }
            }
            else if (object->hasProperty("DEST_X") && object->hasProperty("DEST_Y"))
            {
                // warp to absolute position
                int destX = utils::stringToInt(object->getProperty("DEST_X"));
                int destY = utils::stringToInt(object->getProperty("DEST_Y"));

                action = new WarpAction(destMap, Point(destX, destY));
            }
            else
            {
                LOG_ERROR("Warp \"" << object->getName() << "\" on map "
                          << getName()
                          << " is invalid!");
                continue;
            }

            // add this trigger to the map
            Entity *entity = new Entity(OBJECT_OTHER, this);
            entity->addComponent(
                        new TriggerAreaComponent(
                            sourceBounds,
                            action,
                            false
                        )
                     );
            insert(entity);
        }
        else if (utils::compareStrI(type, "SPAWN") == 0)
        {
            MonsterClass *monster = 0;
            int maxBeings = utils::stringToInt(object->getProperty("MAX_BEINGS"));
            int spawnRate = utils::stringToInt(object->getProperty("SPAWN_RATE"));
            std::string monsterName = object->getProperty("MONSTER_ID");
            int monsterId = utils::stringToInt(monsterName);

            if (monsterId)
            {
                monster = monsterManager->getMonster(monsterId);
                if (!monster)
                {
                    LOG_WARN("Couldn't find monster ID " << monsterId <<
                             " for spawn area");
                }
            }
            else
            {
                monster = monsterManager->getMonsterByName(monsterName);
                if (!monster)
                {
                    LOG_WARN("Couldn't find monster " << monsterName <<
                             " for spawn area");
                }
            }

            if (monster && maxBeings && spawnRate)
            {
                Entity *entity = new Entity(OBJECT_OTHER, this);
                SpawnAreaComponent *spawnArea =
                        new SpawnAreaComponent(monster, object->getBounds(),
                                               maxBeings, spawnRate);

                entity->addComponent(spawnArea);
                insert(entity);
            }
        }
        else if (utils::compareStrI(type, "NPC") == 0)
        {
            int npcId = utils::stringToInt(object->getProperty("NPC_ID"));
            std::string gender = object->getProperty("GENDER");
            std::string scriptText = object->getProperty("SCRIPT");

            if (npcId && !scriptText.empty())
            {
                Script *script = ScriptManager::currentState();
                script->loadNPC(object->getName(), npcId,
                                ManaServ::getGender(gender),
                                object->getX(), object->getY(),
                                scriptText.c_str(), this);
            }
            else
            {
                LOG_WARN("Unrecognized format for npc");
            }
        }
        else if (utils::compareStrI(type, "SCRIPT") == 0)
        {
            std::string scriptFilename = object->getProperty("FILENAME");
            std::string scriptText = object->getProperty("TEXT");

            Script *script = ScriptManager::currentState();
            Script::Context context;
            context.map = this;

            if (!scriptFilename.empty())
            {
                script->loadFile(scriptFilename, context);
            }
            else if (!scriptText.empty())
            {
                std::string name = "'" + object->getName() + "'' in " + mName;
                script->load(scriptText.c_str(), name.c_str(), context);
            }
            else
            {
                LOG_WARN("Unrecognized format for script");
            }
        }
    }
}
