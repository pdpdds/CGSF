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

#ifndef STATE_H
#define STATE_H

#include "utils/point.h"

#include <string>

class Entity;
class ItemClass;
class MapComposite;

namespace GameState
{
    /**
     * Updates game state (contains core server logic).
     */
    void update(int tick);

    int getCurrentTick();

    /**
     * Inserts an entity in the game world.
     * @return false if the insertion failed and the entity is in limbo.
     * @note No update may be in progress.
     */
    bool insert(Entity *)
#   ifdef __GNUC__
    __attribute__((warn_unused_result))
#   endif
    ;

    /**
     * Inserts a entity in the game world. Deletes the entity if the insertion
     * failed.
     * @return false if the insertion failed.
     * @note No update may be in progress. Invalid for characters.
     */
    bool insertOrDelete(Entity *);

    /**
     * Removes a entity from the game world.
     * @note No update may be in progress.
     * @note The entity is not destroyed by this call.
     */
    void remove(Entity *);

    /**
     * Warps a character between places of the game world.
     * @note No update may be in progress.
     * @note The character is destroyed, if needed.
     */
    void warp(Entity *, MapComposite *, const Point &point);

    /**
     * Enqueues an insert event.
     * @note The event will be executed at end of update.
     */
    void enqueueInsert(Entity *);

    /**
     * Enqueues a remove event.
     * @note The event will be executed at end of update.
     * @note The entity will be destroyed at that time.
     */
    void enqueueRemove(Entity *);

    /**
     * Enqueues a warp event.
     * @note The event will be executed at end of update.
     */
    void enqueueWarp(Entity *, MapComposite *, const Point &point);

    /**
     * Says something to an actor.
     * @note passing nullptr as source generates a message from "Server:"
     */
    void sayTo(Entity *destination, Entity *source, const std::string &text);

    /**
     * Says something to everything around an actor.
     */
    void sayAround(Entity *entity, const std::string &text);

    /**
     * Says something to every player on the server.
     */
    void sayToAll(const std::string &text);

    /**
     * Gets the cached value of a global script variable.
     */
    std::string getVariable(const std::string &key);

    /**
     * Changes a global script variable and notifies the database server
     * about the change.
     */
    void setVariable(const std::string &key, const std::string &value);

    /**
     * Changes a global variable without notifying the database server
     * about the change.
     */
    void setVariableFromDbserver(const std::string &key, const std::string &value);

    /**
     * Informs all maps about the change of a variable so the maps can call
     * callbacks for those.
     */
    void callVariableCallbacks(const std::string &key,
                               const std::string &value);
}

#endif // STATE_H
