/*
 *  The Mana Server
 *  Copyright (C) 2007-2010  The Mana World Development Team
 *  Copyright (C) 2010  The Mana Developers
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

#include "luautil.h"

#include <string.h>

#include "charactercomponent.h"
#include "itemmanager.h"
#include "monster.h"
#include "monstermanager.h"
#include "npc.h"

#include "utils/logger.h"

#include "scripting/luascript.h"


void raiseWarning(lua_State *, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    char message[1024];
    vsprintf(message, format, args);
    va_end( args );

    LOG_WARN("Lua script error: " << message);
}


bool UserDataCache::retrieve(lua_State *s)
{
    // Retrieve the cache table
    lua_pushlightuserdata(s, &mRegistryKey);    // object_key, key
    lua_rawget(s, LUA_REGISTRYINDEX);           // object_key, Cache?

    if (lua_isnil(s, -1))
    {
        lua_pop(s, 1);
        return false;
    }

    lua_pushvalue(s, -2);                       // object_key, Cache, object_key
    lua_rawget(s, -2);                          // object_key, Cache, UD?

    if (lua_isnil(s, -1))
    {
        lua_pop(s, 2);                          // object_key
        return false;
    }

    lua_replace(s, -3);                         // UD, Cache
    lua_pop(s, 1);                              // UD
    return true;
}

void UserDataCache::insert(lua_State *s)
{
    // Retrieve the cache table
    lua_pushlightuserdata(s, &mRegistryKey);    // object_key, UD, key
    lua_rawget(s, LUA_REGISTRYINDEX);           // object_key, UD, Cache?

    // Create the cache when it doesn't exist yet
    if (lua_isnil(s, -1))
    {
        lua_pop(s, 1);                          // object_key, UD
        lua_newtable(s);                        // object_key, UD, Cache

        // The metatable that makes the values in the table above weak
        lua_createtable(s, 0, 1);               // object_key, UD, Cache, {}
        lua_pushliteral(s, "__mode");
        lua_pushliteral(s, "v");
        lua_rawset(s, -3);                      // object_key, UD, Cache, { __mode = "v" }
        lua_setmetatable(s, -2);                // object_key, UD, Cache

        lua_pushlightuserdata(s, &mRegistryKey);// object_key, UD, Cache, key
        lua_pushvalue(s, -2);                   // object_key, UD, Cache, key, Cache
        lua_rawset(s, LUA_REGISTRYINDEX);       // object_key, UD, Cache
    }

    lua_pushvalue(s, -3);                       // object_key, UD, Cache, object_key
    lua_pushvalue(s, -3);                       // object_key, UD, Cache, object_key, UD
    lua_rawset(s, -3);                          // object_key, UD, Cache { object_key = UD }
    lua_pop(s, 1);                              // object_key, UD
    lua_replace(s, -2);                         // UD
}


UserDataCache LuaUserData<Entity>::mUserDataCache;

void LuaUserData<Entity>::registerType(lua_State *s, const luaL_Reg *members)
{
    luaL_newmetatable(s, "Entity");         // metatable
    lua_pushliteral(s, "__index");          // metatable, "__index"
    lua_createtable(s, 0, 0);               // metatable, "__index", {}
#if LUA_VERSION_NUM < 502
    luaL_register(s, nullptr, members);
#else
    luaL_setfuncs(s, members, 0);
#endif

    // Make the functions table available as a global
    lua_pushvalue(s, -1);                   // metatable, "__index", {}, {}
    lua_setglobal(s, "Entity");             // metatable, "__index", {}

    lua_rawset(s, -3);                      // metatable
    lua_pop(s, 1);                          // -empty-
}

void LuaUserData<Entity>::push(lua_State *s, Entity *entity)
{
    if (!entity)
    {
        lua_pushnil(s);
    }
    else
    {
#if LUA_VERSION_NUM < 502
        lua_pushnumber(s, entity->getId());
#else
        lua_pushunsigned(s, entity->getId());
#endif

        if (!mUserDataCache.retrieve(s))
        {
            void *userData = lua_newuserdata(s, sizeof(unsigned));
            * static_cast<unsigned*>(userData) = entity->getId();

#if LUA_VERSION_NUM < 502
            luaL_newmetatable(s, "Entity");
            lua_setmetatable(s, -2);
#else
            luaL_setmetatable(s, "Entity");
#endif

            mUserDataCache.insert(s);
        }
    }
}

Entity *LuaUserData<Entity>::check(lua_State *L, int narg)
{
    void *userData = luaL_checkudata(L, narg, "Entity");
    Entity *entity = findEntity(*(static_cast<unsigned*>(userData)));
    luaL_argcheck(L, entity, narg, "invalid entity");
    return entity;
}


Script *getScript(lua_State *s)
{
    lua_pushlightuserdata(s, (void *)&LuaScript::registryKey);
    lua_gettable(s, LUA_REGISTRYINDEX);
    Script *script = static_cast<Script *>(lua_touserdata(s, -1));
    lua_pop(s, 1);
    return script;
}


/* Functions below are unsafe, as they assume the script has passed pointers
   to objects which have not yet been destroyed. If the script never keeps
   pointers around, there will be no problem. In order to be safe, the engine
   should replace pointers by local identifiers and store them in a map. By
   listening to the death of objects, it could keep track of pointers still
   valid in the map.
   TODO: do it. */

ItemClass *getItemClass(lua_State *s, int p)
{
    ItemClass *itemClass = 0;

    switch (lua_type(s, p))
    {
    case LUA_TNUMBER:
        itemClass = itemManager->getItem(lua_tointeger(s, p));
        break;
    case LUA_TSTRING:
        itemClass = itemManager->getItemByName(lua_tostring(s, p));
        break;
    case LUA_TUSERDATA:
        itemClass = LuaItemClass::check(s, p);
        break;
    }

    return itemClass;
}

MonsterClass *getMonsterClass(lua_State *s, int p)
{
    MonsterClass *monsterClass = 0;

    switch (lua_type(s, p))
    {
    case LUA_TNUMBER:
        monsterClass = monsterManager->getMonster(lua_tointeger(s, p));
        break;
    case LUA_TSTRING:
        monsterClass = monsterManager->getMonsterByName(lua_tostring(s, p));
        break;
    case LUA_TUSERDATA:
        monsterClass = LuaMonsterClass::check(s, p);
        break;
    }

    return monsterClass;
}


bool checkOptionalBool(lua_State *s, int p, bool defaultValue)
{
    if (lua_gettop(s) >= p)
    {
        luaL_argcheck(s, lua_isboolean(s, p), p, "boolean expected");
        return lua_toboolean(s, p);
    }
    return defaultValue;
}

Entity *checkActor(lua_State *s, int p)
{
    Entity *entity = LuaEntity::check(s, p);
    luaL_argcheck(s, entity->hasComponent<ActorComponent>(), p,
                  "entity has no actor component");
    return entity;
}

Entity *checkBeing(lua_State *s, int p)
{
    Entity *entity = LuaEntity::check(s, p);
    luaL_argcheck(s, entity->hasComponent<BeingComponent>(), p,
                  "entity has no being component");
    return entity;
}

Entity *checkCharacter(lua_State *s, int p)
{
    Entity *entity = LuaEntity::check(s, p);
    luaL_argcheck(s, entity->getType() == OBJECT_CHARACTER, p, "character expected");
    return entity;
}

ItemClass *checkItemClass(lua_State *s, int p)
{
    ItemClass *itemClass = getItemClass(s, p);
    luaL_argcheck(s, itemClass, p, "item type expected");
    return itemClass;
}

Entity *checkMonster(lua_State *s, int p)
{
    Entity *entity = LuaEntity::check(s, p);
    luaL_argcheck(s, entity->getType() == OBJECT_MONSTER, p, "monster expected");
    return entity;
}

MonsterClass *checkMonsterClass(lua_State *s, int p)
{
    MonsterClass *monsterClass = getMonsterClass(s, p);
    luaL_argcheck(s, monsterClass, p, "monster type expected");
    return monsterClass;
}

Entity *checkNpc(lua_State *s, int p)
{
    Entity *entity = LuaEntity::check(s, p);
    luaL_argcheck(s, entity->getType() == OBJECT_NPC, p, "npc expected");
    return entity;
}

AbilityManager::AbilityInfo *checkAbility(lua_State *s, int p)
{
    AbilityManager::AbilityInfo *abilityInfo;
    if (lua_isnumber(s, p))
        abilityInfo = abilityManager->getAbilityInfo(luaL_checkint(s, p));
    else
        abilityInfo = abilityManager->getAbilityInfo(luaL_checkstring(s, p));

    luaL_argcheck(s, abilityInfo != nullptr, p, "invalid ability");
    return abilityInfo;
}

AttributeInfo *checkAttribute(lua_State *s, int p)
{
    AttributeInfo *attributeInfo = 0;

    switch (lua_type(s, p))
    {
    case LUA_TNUMBER:
        attributeInfo = attributeManager->getAttributeInfo(luaL_checkint(s, p));
        break;
    case LUA_TSTRING:
        attributeInfo = attributeManager->getAttributeInfo(luaL_checkstring(s, p));
        break;
    case LUA_TUSERDATA:
        attributeInfo = LuaAttributeInfo::check(s, p);
        break;
    }

    luaL_argcheck(s, attributeInfo != nullptr, p, "invalid attribute");
    return attributeInfo;
}

unsigned char checkWalkMask(lua_State *s, int p)
{
    const char *stringMask = luaL_checkstring(s, p);
    unsigned char mask = 0x00;
    if (strchr(stringMask, 'w'))
        mask |= Map::BLOCKMASK_WALL;
    if (strchr(stringMask, 'c'))
        mask |= Map::BLOCKMASK_CHARACTER;
    if (strchr(stringMask, 'm'))
        mask |= Map::BLOCKMASK_MONSTER;

    return mask;
}


MapComposite *checkCurrentMap(lua_State *s, Script *script /* = 0 */)
{
    if (!script)
        script = getScript(s);

    MapComposite *mapComposite = script->getContext()->map;
    if (!mapComposite)
        luaL_error(s, "no current map");

    return mapComposite;
}

Script::Thread *checkCurrentThread(lua_State *s, Script *script /* = 0 */)
{
    if (!script)
        script = getScript(s);

    Script::Thread *thread = script->getCurrentThread();
    if (!thread)
        luaL_error(s, "function requires threaded execution");

    return thread;
}
