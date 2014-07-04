/*
 *  The Mana Server
 *  Copyright (C) 2007-2010  The Mana World Development Team
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

#ifndef LUASCRIPT_H
#define LUASCRIPT_H

extern "C" {
#include <lualib.h>
#include <lauxlib.h>
}

#include "scripting/script.h"

class CharacterComponent;

/**
 * Implementation of the Script class for Lua.
 */
class LuaScript : public Script
{
    public:
        /**
         * Constructor. Initializes a new Lua state, registers the native API
         * and loads the libmana.lua file.
         */
        LuaScript();

        ~LuaScript();

        void load(const char *prog, const char *name,
                  const Context &context = Context());

        Thread *newThread();

        void prepare(Ref function);

        void prepareResume(Thread *thread);

        void push(int);
        void push(const std::string &);
        void push(Entity *);
        void push(const std::list<InventoryItem> &itemList);
        void push(AttributeInfo *);

        int execute(const Context &context = Context());

        bool resume();

        void assignCallback(Ref &function);

        void unref(Ref &ref);

        static void getQuestCallback(Entity *,
                                     const std::string &value,
                                     Script *);

        static void getPostCallback(Entity *,
                                    const std::string &sender,
                                    const std::string &letter,
                                    Script *);

        void processDeathEvent(Entity *entity);

        void processRemoveEvent(Entity *entity);


        static void setDeathNotificationCallback(Script *script)
        { script->assignCallback(mDeathNotificationCallback); }

        static void setRemoveNotificationCallback(Script *script)
        { script->assignCallback(mRemoveNotificationCallback); }

        static const char registryKey;

    private:
        class LuaThread : public Thread
        {
            public:
                LuaThread(LuaScript *script);
                ~LuaThread();

                lua_State *mState;
                int mRef;
        };

        lua_State *mRootState;
        lua_State *mCurrentState;
        int nbArgs;

        static Ref mDeathNotificationCallback;
        static Ref mRemoveNotificationCallback;

        friend class LuaThread;
};

static Script *LuaFactory()
{
    return new LuaScript();
}

struct LuaRegister
{
    LuaRegister() { Script::registerEngine("lua", LuaFactory); }
};

//static LuaRegister dummy;

#endif // LUASCRIPT_H
