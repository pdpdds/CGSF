/*
 *  The Mana Server
 *  Copyright (C) 2004-2010  The Mana World Development Team
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

#ifndef EFFECT_H
#define EFFECT_H

#include "component.h"
#include "state.h"

class Entity;
class MapComposite;
class Point;

class EffectComponent : public Component
{
    public:
        static const ComponentType type = CT_Effect;

        EffectComponent(int id)
          : mEffectId(id)
          , mBeing(0)
        {}

        int getEffectId() const
        { return mEffectId; }

        Entity *getBeing() const
        { return mBeing; }

        /**
         * Removes effect after it has been shown.
         */
        void update(Entity &entity);

        void setBeing(Entity *b)
        { mBeing = b; }

    private:
        int mEffectId;
        Entity *mBeing;
};


namespace Effects
{
    /**
     * Convenience methods to show an effect.
     */
    void show(int id, MapComposite *map, const Point &pos);
    void show(int id, Entity *b);

    // TODO: get this in sync with effects.xml
    enum {
       FIRE_BURST        = 15
    };
}

#endif // EFFECT_H
