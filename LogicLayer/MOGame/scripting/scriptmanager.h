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

#ifndef SCRIPTMANAGER_H
#define SCRIPTMANAGER_H

#include "charactercomponent.h"

#include <string>

class Script;

/**
 * Manages the script states. In fact at the moment it simply provides access
 * to the single global script state, but in the future it is planned to allow
 * reloading the scripts while the server is running, by keeping old script
 * states around until they are no longer in use.
 */
namespace ScriptManager {

/**
 * Initializes the script manager by creating the script state.
 */
void initialize();

/**
 * Deinitializes the script manager by deleting the script state.
 */
void deinitialize();

/**
 * Loads the main script file.
 */
bool loadMainScript(const std::string &file);

/**
 * Returns the current global script state.
 */
Script *currentState();

bool performCraft(Entity *crafter, const std::list<InventoryItem> &recipe);

void setCraftCallback(Script *script);
void setAbilityCallback(Script *script);
void setGetAbilityRechargeCostCallback(Script *script);

} // namespace ScriptManager

#endif // SCRIPTMANAGER_H
