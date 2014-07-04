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

#include "scripting/script.h"

#include "common/configuration.h"
#include "common/resourcemanager.h"
#include "being.h"
#include "utils/logger.h"

#include <cassert>
#include <cstdlib>
#include <map>

#include <string.h>

typedef std::map< std::string, Script::Factory > Engines;

static Engines *engines = nullptr;

Script::Ref Script::mCreateNpcDelayedCallback;
Script::Ref Script::mUpdateCallback;

Script::Script():
    mCurrentThread(0),
    mContext(0)
{}

Script::~Script()
{
    // There should be no remaining threads when the Script gets deleted
    assert(mThreads.empty());
}

void Script::registerEngine(const std::string &name, Factory f)
{
    if (!engines)
    {
        /* The "engines" variable is a pointer instead of an object, because
           this file may not have been initialized at the time this function
           is called. So we take care of the initialization by hand, in order
           to ensure we will not segfault at startup. */
        engines = new Engines;
    }
    (*engines)[name] = f;
}

Script *Script::create(const std::string &engine)
{
    if (engines)
    {
        Engines::const_iterator i = engines->find(engine);
        if (i != engines->end())
        {
            return i->second();
        }
    }
    LOG_ERROR("No scripting engine named " << engine);
    return nullptr;
}

void Script::update()
{
    if (!mUpdateCallback.isValid())
    {
        LOG_ERROR("Could not find callback for update function!");
        return;
    }
    prepare(mUpdateCallback);
    execute();
}

static char *skipPotentialBom(char *text)
{
    // Based on the C version of bomstrip
    const char * const utf8Bom = "\xef\xbb\xbf";
    const int bomLength = strlen(utf8Bom);
    return (strncmp(text, utf8Bom, bomLength) == 0) ? text + bomLength : text;
}

bool Script::loadFile(const std::string &name, const Context &context)
{
    int size;
    char *buffer = ResourceManager::loadFile(name, size);
    if (buffer)
    {
        mScriptFile = name;
        load(skipPotentialBom(buffer), name.c_str(), context);
        free(buffer);
        return true;
    } else {
        return false;
    }
}

void Script::loadNPC(const std::string &name,
                     int id,
                     ManaServ::BeingGender gender,
                     int x, int y,
                     const char *prog,
                     MapComposite *map)
{
    if (!mCreateNpcDelayedCallback.isValid())
    {
        LOG_ERROR("No callback for creating npcs delayed assigned. "
                  "Could not enabled NPC");
        return;
    }
    Context context;
    context.map = map;
    load(prog, name.c_str(), context);
    prepare(mCreateNpcDelayedCallback);
    push(name);
    push(id);
    push(gender);
    push(x);
    push(y);
    execute(context);
}

int Script::execute(MapComposite *map)
{
    Context context;
    context.map = map;
    return execute(context);
}


/**
 * Removes one element matching the given value by overwriting it with the last
 * element and then popping the last element.
 */
template<typename T>
static void fastRemoveOne(std::vector<T> &vector, T value)
{
    for (size_t i = 0, size = vector.size(); i < size; ++i)
    {
        if (vector.at(i) == value)
        {
            vector.at(i) = vector.back();
            vector.pop_back();
            break;
        }
    }
}

Script::Thread::Thread(Script *script) :
    mScript(script),
    mState(ThreadPending)
{
    script->mThreads.push_back(this);
}

Script::Thread::~Thread()
{
    fastRemoveOne(mScript->mThreads, this);
}
