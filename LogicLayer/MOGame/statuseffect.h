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

#ifndef STATUSEFFECT_H
#define STATUSEFFECT_H

#include "scripting/script.h"

class Entity;

class StatusEffect
{
    public:
        StatusEffect(int id);
        ~StatusEffect();

        void tick(Entity &target, int count);

        int getId() const
        { return mId; }

        void setTickCallback(Script *script)
        { script->assignCallback(mTickCallback); }

    private:
        int mId;
        Script::Ref mTickCallback;
};

#endif
