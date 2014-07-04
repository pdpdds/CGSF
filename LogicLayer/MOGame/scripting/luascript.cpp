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

#include "luascript.h"

#include "scripting/luautil.h"
#include "scripting/scriptmanager.h"

#include "charactercomponent.h"
#include "utils/logger.h"

#include <cassert>
#include <cstring>

Script::Ref LuaScript::mDeathNotificationCallback;
Script::Ref LuaScript::mRemoveNotificationCallback;

const char LuaScript::registryKey = 0;

LuaScript::~LuaScript()
{
    lua_close(mRootState);
}

void LuaScript::prepare(Ref function)
{
    assert(nbArgs == -1);

    assert(function.isValid());
    lua_rawgeti(mCurrentState, LUA_REGISTRYINDEX, function.value);
    assert(lua_isfunction(mCurrentState, -1));
    nbArgs = 0;
}

Script::Thread *LuaScript::newThread()
{
    assert(nbArgs == -1);
    assert(!mCurrentThread);

    LuaThread *thread = new LuaThread(this);

    mCurrentThread = thread;
    mCurrentState = thread->mState;
    return thread;
}

void LuaScript::prepareResume(Thread *thread)
{
    assert(nbArgs == -1);
    assert(!mCurrentThread);

    mCurrentThread = thread;
    mCurrentState = static_cast<LuaThread*>(thread)->mState;
    nbArgs = 0;
}

void LuaScript::push(int v)
{
    assert(nbArgs >= 0);
    ::push(mCurrentState, v);
    ++nbArgs;
}

void LuaScript::push(const std::string &v)
{
    assert(nbArgs >= 0);
    ::push(mCurrentState, v);
    ++nbArgs;
}

void LuaScript::push(Entity *v)
{
    assert(nbArgs >= 0);
    ::push(mCurrentState, v);
    ++nbArgs;
}

void LuaScript::push(const std::list<InventoryItem> &itemList)
{
    assert(nbArgs >= 0);
    int position = 0;

    lua_createtable(mCurrentState, itemList.size(), 0);
    int itemTable = lua_gettop(mCurrentState);

    for (const InventoryItem &inventoryItem : itemList)
    {
        // create the item structure
        std::map<std::string, int> item;
        item["id"] = inventoryItem.itemId;
        item["amount"] = inventoryItem.amount;
        pushSTLContainer<std::string, int>(mCurrentState, item);
        lua_rawseti(mCurrentState, itemTable, ++position);
    }
    ++nbArgs;
}

void LuaScript::push(AttributeInfo *attributeInfo)
{
    assert(nbArgs >= 0);
    ::push(mCurrentState, attributeInfo);
    ++nbArgs;
}

int LuaScript::execute(const Context &context)
{
    assert(nbArgs >= 0);

    const Context *previousContext = mContext;
    mContext = &context;

    const int tmpNbArgs = nbArgs;
    nbArgs = -1;
    int res = lua_pcall(mCurrentState, tmpNbArgs, 1, 1);

    if (res || !(lua_isnil(mCurrentState, -1) || lua_isnumber(mCurrentState, -1)))
    {
        const char *s = lua_tostring(mCurrentState, -1);

        LOG_WARN("Lua Script Error" << std::endl
                 << "     Script  : " << mScriptFile << std::endl
                 << "     Error   : " << (s ? s : "") << std::endl);
        lua_pop(mCurrentState, 1);
        return 0;
    }
    res = lua_tointeger(mCurrentState, -1);
    lua_pop(mCurrentState, 1);
    mContext = previousContext;
    return res;
}

bool LuaScript::resume()
{
    assert(nbArgs >= 0);
    assert(mCurrentThread);

    const Context *previousContext = mContext;
    mContext = &mCurrentThread->getContext();

    const int tmpNbArgs = nbArgs;
    nbArgs = -1;
#if LUA_VERSION_NUM < 502
    int result = lua_resume(mCurrentState, tmpNbArgs);
#else
    int result = lua_resume(mCurrentState, nullptr, tmpNbArgs);
#endif

    if (result == 0)                // Thread is done
    {
        if (lua_gettop(mCurrentState) > 0)
            LOG_WARN("Ignoring values returned by script thread!");
    }
    else if (result == LUA_YIELD)   // Thread has yielded
    {
        if (lua_gettop(mCurrentState) > 0)
            LOG_WARN("Ignoring values passed to yield!");
    }
    else                            // Thread encountered an error
    {
        // Make a traceback using the debug.traceback function
        lua_getglobal(mCurrentState, "debug");
        lua_getfield(mCurrentState, -1, "traceback");
        lua_pushvalue(mCurrentState, -3); // error string as first parameter
        lua_pcall(mCurrentState, 1, 1, 0);

        LOG_WARN("Lua Script Error:" << std::endl
                 << lua_tostring(mCurrentState, -1));
    }

    lua_settop(mCurrentState, 0);
    mContext = previousContext;
    const bool done = result != LUA_YIELD;

    if (done)
    {
        // Clean up the current thread (not sure if this is the best place)
        delete mCurrentThread;
    }

    mCurrentThread = 0;
    mCurrentState = mRootState;

    return done;
}

void LuaScript::assignCallback(Script::Ref &function)
{
    assert(lua_isfunction(mRootState, -1));

    // If there is already a callback set, replace it
    if (function.isValid())
        luaL_unref(mRootState, LUA_REGISTRYINDEX, function.value);

    function.value = luaL_ref(mRootState, LUA_REGISTRYINDEX);
}

void LuaScript::unref(Ref &ref)
{
    if (ref.isValid())
    {
        luaL_unref(mRootState, LUA_REGISTRYINDEX, ref.value);
        ref.value = -1;
    }
}

void LuaScript::load(const char *prog, const char *name,
                     const Context &context)
{
    const Context *previousContext = mContext;
    mContext = &context;
    int res = luaL_loadbuffer(mRootState, prog, std::strlen(prog), name);
    if (res)
    {
        switch (res) {
        case LUA_ERRSYNTAX:
            LOG_ERROR("Syntax error while loading Lua script: "
                      << lua_tostring(mRootState, -1));
            break;
        case LUA_ERRMEM:
            LOG_ERROR("Memory allocation error while loading Lua script");
            break;
        }

        lua_pop(mRootState, 1);
    }
    else if (lua_pcall(mRootState, 0, 0, 1))
    {
        LOG_ERROR("Failure while initializing Lua script: "
                  << lua_tostring(mRootState, -1));
        lua_pop(mRootState, 1);
    }
    mContext = previousContext;
}

void LuaScript::processDeathEvent(Entity *entity)
{
    if (mDeathNotificationCallback.isValid())
    {
        prepare(mDeathNotificationCallback);
        push(entity);
        //TODO: get and push a list of creatures who contributed to killing the
        //      being. This might be very interesting for scripting quests.
        Script::execute(entity->getMap());
    }
}

void LuaScript::processRemoveEvent(Entity *entity)
{
    if (mRemoveNotificationCallback.isValid())
    {
        prepare(mRemoveNotificationCallback);
        push(entity);
        //TODO: get and push a list of creatures who contributed to killing the
        //      being. This might be very interesting for scripting quests.
        Script::execute(entity->getMap());
    }
}

/**
 * Called when the server has recovered the value of a quest variable.
 */
void LuaScript::getQuestCallback(Entity *q,
                                 const std::string &value,
                                 Script *script)
{
    auto *characterComponent = q->getComponent<CharacterComponent>();
    Script::Thread *thread = characterComponent->getNpcThread();
    if (!thread || thread->mState != Script::ThreadExpectingString)
        return;

    script->prepareResume(thread);
    script->push(value);
    characterComponent->resumeNpcThread();
}

/**
 * Called when the server has recovered the post for a user.
 */
void LuaScript::getPostCallback(Entity *q,
                                const std::string &sender,
                                const std::string &letter,
                                Script *script)
{
    auto *characterComponent = q->getComponent<CharacterComponent>();
    Script::Thread *thread = characterComponent->getNpcThread();
    if (!thread || thread->mState != Script::ThreadExpectingTwoStrings)
        return;

    script->prepareResume(thread);
    script->push(sender);
    script->push(letter);
    characterComponent->resumeNpcThread();
}


LuaScript::LuaThread::LuaThread(LuaScript *script) :
    Thread(script)
{
    mState = lua_newthread(script->mRootState);
    mRef = luaL_ref(script->mRootState, LUA_REGISTRYINDEX);
}

LuaScript::LuaThread::~LuaThread()
{
    LuaScript *luaScript = static_cast<LuaScript*>(mScript);
    luaL_unref(luaScript->mRootState, LUA_REGISTRYINDEX, mRef);
}
