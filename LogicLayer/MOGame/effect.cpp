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

#include "effect.h"

#include "being.h"
#include "entity.h"
#include "state.h"

void EffectComponent::update(Entity &entity)
{
    GameState::enqueueRemove(&entity);
}

namespace Effects
{
    void show(int id, MapComposite *map, const Point &pos)
    {
        Entity *effect = new Entity(OBJECT_EFFECT);
        auto *actorComponent = new ActorComponent(*effect);
        effect->addComponent(actorComponent);
        effect->addComponent(new EffectComponent(id));
        effect->setMap(map);
        actorComponent->setPosition(*effect, pos);

        GameState::enqueueInsert(effect);
    }

    void show(int id, Entity *b)
    {
        EffectComponent *effectComponent = new EffectComponent(id);
        effectComponent->setBeing(b);

        Entity *effect = new Entity(OBJECT_EFFECT);
        auto *actorComponent = new ActorComponent(*effect);
        effect->addComponent(actorComponent);
        effect->addComponent(effectComponent);
        effect->setMap(b->getMap());
        const Point &point = b->getComponent<ActorComponent>()->getPosition();
        actorComponent->setPosition(*effect, point);

        GameState::enqueueInsert(effect);
    }
}
