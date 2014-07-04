/*
 *  The Mana Server
 *  Copyright (C) 2006-2010  The Mana World Development Team
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

#include "triggerareacomponent.h"

#include "charactercomponent.h"
#include "mapcomposite.h"
#include "actorcomponent.h"
#include "state.h"

#include "utils/logger.h"

#include <cassert>

void WarpAction::process(Entity *obj)
{
    if (obj->getType() == OBJECT_CHARACTER)
    {
        GameState::enqueueWarp(obj, mMap, mTargetPoint);
    }
}

void AutowarpAction::process(Entity *obj)
{
    Point targetPoint;

    // only characters can warp
    if (obj->getType() != OBJECT_CHARACTER)
        return;

    // get the direction
    ActorComponent *actor = obj->getComponent<ActorComponent>();

    // calculate proportions
    float horizontal = float(mTargetArea.w) / float(mSourceArea.w);
    float vertical = float(mTargetArea.h) / float(mSourceArea.h);

    // calculate final target position
    const Point &actorPosition = actor->getPosition();
    int actorSize = actor->getSize();
    switch (mDirection)
    {
    case ExitNorth:
        targetPoint.x = (actorPosition.x - mSourceArea.x) * horizontal + mTargetArea.x;
        targetPoint.y = mTargetArea.y - actorSize;
        break;
    case ExitSouth:
        targetPoint.x = (actorPosition.x - mSourceArea.x) * horizontal + mTargetArea.x;
        targetPoint.y = mTargetArea.y + mTargetArea.h + actorSize;
        break;
    case ExitEast:
        targetPoint.x = mTargetArea.x + mTargetArea.w + actorSize;
        targetPoint.y = (actorPosition.y - mSourceArea.y) * vertical + mTargetArea.y;
        break;
    case ExitWest:
        targetPoint.x = mTargetArea.x - actorSize;
        targetPoint.y = (actorPosition.y - mSourceArea.y) * vertical + mTargetArea.y;
        break;
    }

    GameState::enqueueWarp(obj, mMap, targetPoint);
}

ScriptAction::ScriptAction(Script *script, Script::Ref callback, int arg) :
    mScript(script),
    mCallback(callback),
    mArg(arg)
{
    assert(mCallback.isValid());
}

void ScriptAction::process(Entity *obj)
{
    LOG_DEBUG("Script trigger area activated: "
              << "(" << obj << ", " << mArg << ")");

    mScript->prepare(mCallback);
    mScript->push(obj);
    mScript->push(mArg);
    mScript->execute(obj->getMap());
}

void TriggerAreaComponent::update(Entity &entity)
{
    MapComposite *map = entity.getMap();
    std::set<Entity *> insideNow;

    for (BeingIterator i(map->getInsideRectangleIterator(mZone)); i; ++i)
    {
        // Don't deal with uninitialized actors
        if (!(*i) || !(*i)->getComponent<ActorComponent>()->isPublicIdValid())
            continue;

        // The BeingIterator returns the mapZones in touch with the rectangle
        // area. On the other hand, the beings contained in the map zones
        // may not be within the rectangle area. Hence, this additional
        // contains() condition.
        const Point &point =
                (*i)->getComponent<ActorComponent>()->getPosition();
        if (mZone.contains(point))
        {
            insideNow.insert(*i);

            if (!mOnce || mInside.find(*i) == mInside.end())
            {
                mAction->process(*i);
            }
        }
    }
    mInside.swap(insideNow); //swapping is faster than assigning
}
