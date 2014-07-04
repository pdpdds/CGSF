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

#include "scriptmanager.h"

#include "common/configuration.h"
#include "scripting/script.h"
#include "scripting/luascript.h"

static Script *_currentState;

static Script::Ref _craftCallback;

LuaRegister dummy;

void ScriptManager::initialize()
{
	
    const std::string engine = Configuration::getValue("script_engine", "lua");
    _currentState = Script::create(engine);
}

void ScriptManager::deinitialize()
{
    delete _currentState;
    _currentState = 0;
}

bool ScriptManager::loadMainScript(const std::string &file)
{
    return _currentState->loadFile(file);
}

Script *ScriptManager::currentState()
{
    return _currentState;
}

bool ScriptManager::performCraft(Entity *crafter,
                                 const std::list<InventoryItem> &recipe)
{
    if (!_craftCallback.isValid())
    {
        LOG_WARN("No crafting callback set! Crafting disabled.");
        return false;
    }
    _currentState->prepare(_craftCallback);
    _currentState->push(crafter);
    _currentState->push(recipe);
    _currentState->execute(crafter->getMap());
    return true;
}

void ScriptManager::setCraftCallback(Script *script)
{ script->assignCallback(_craftCallback); }
