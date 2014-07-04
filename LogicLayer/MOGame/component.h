/*
 *  The Mana Server
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

#ifndef COMPONENT_H
#define COMPONENT_H

#include <sigc++/trackable.h>

class Entity;

enum ComponentType
{
    CT_Ability,
    CT_Actor,
    CT_Character,
    CT_Being,
    CT_Effect,
    CT_Fighting,
    CT_Item,
    CT_Monster,
    CT_Npc,
    CT_SpawnArea,
    CT_TriggerArea,

    ComponentTypeCount
};

/**
 * A component of an entity.
 */
class Component : public sigc::trackable
{
public:
    Component() {}
    Component(const Component &rhs) = delete;
    Component &operator=(const Component &rhs) = delete;
    virtual ~Component() {}

    /**
     * Updates the internal status. The \a entity is the owner of this
     * component.
     */
    virtual void update(Entity &entity) = 0;
};

#endif // COMPONENT_H
