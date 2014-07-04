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

#ifndef SCRIPTING_SCRIPT_H
#define SCRIPTING_SCRIPT_H

#include "common/inventorydata.h"
#include "common/manaserv_protocol.h"

#include "attributemanager.h"

#include <list>
#include <string>
#include <vector>
#include <stack>

#include <sigc++/trackable.h>

class MapComposite;
class Entity;

/**
 * Abstract interface for calling functions written in an external language.
 */
class Script : public sigc::trackable
{
    public:
        struct Context
        {
            MapComposite *map;
            Entity *npc;
            Entity *character;

            Context()
                : map(0)
                , npc(0)
                , character(0)
            {}
        };

        /**
         * Defines a function that creates a Script instance.
         */
        typedef Script *(*Factory)();

        /**
         * Registers a new scripting engine.
         */
        static void registerEngine(const std::string &, Factory);

        /**
         * Creates a new script context for a given engine.
         */
        static Script *create(const std::string &engine);

        /**
         * A reference to a script object. It wraps an integer value, but adds
         * custom initialization and a definition of valid. It also makes the
         * purpose clear.
         */
        class Ref
        {
            public:
                Ref() : value(-1) {}
                Ref(int value) : value(value) {}
                bool isValid() const { return value != -1; }
                int value;
        };

        enum ThreadState {
            ThreadPending,
            ThreadPaused,
            ThreadExpectingNumber,
            ThreadExpectingString,
            ThreadExpectingTwoStrings
        };

        /**
         * A script thread. Meant to be extended by the Script subclass to
         * store additional information.
         */
        class Thread
        {
            public:
                Thread(Script *script);
                virtual ~Thread();

                Context &getContext()
                { return mContext; }

                Script * const mScript;
                ThreadState mState;
                Context mContext;
        };

        Script();

        virtual ~Script();

        /**
         * Loads a chunk of text into script context and executes its global
         * statements.
         *
         * @param prog the program text to load
         * @param name the name of the text, used for error reporting
         * @param context the context that is supposed to be used for loading
         */
        virtual void load(const char *prog,
                          const char *name,
                          const Context &context = Context()) = 0;

        /**
         * Loads a text file into script context and executes its global
         * statements.
         */
        virtual bool loadFile(const std::string &,
                              const Context &context = Context());

        /**
         * Loads a chunk of text and considers it as an NPC handler. This
         * handler will later be used to create the given NPC.
         */
        virtual void loadNPC(const std::string &name,
                             int id,
                             ManaServ::BeingGender gender,
                             int x, int y,
                             const char *,
                             MapComposite *map);

        /**
         * Called every tick for the script to manage its data.
         * Calls the "update" function of the script by default.
         */
        virtual void update();

        /**
         * Creates a new script thread and makes it the current one. Script
         * threads do not execute in parallel, but they can suspend execution
         * and be resumed later.
         *
         * The new thread should be prepared as usual, but instead of
         * execute(), the resume() function should be called.
         */
        virtual Thread *newThread() = 0;

        /**
         * Prepares a call to the referenced function.
         * Only one function can be prepared at once.
         */
        virtual void prepare(Ref function) = 0;

        /**
         * Prepares for resuming the given script thread.
         * Only one thread can be resumed at once.
         */
        virtual void prepareResume(Thread *thread) = 0;

        /**
         * Pushes an integer argument for the function being prepared.
         */
        virtual void push(int) = 0;

        /**
         * Pushes a string argument for the function being prepared.
         */
        virtual void push(const std::string &) = 0;

        /**
         * Pushes a pointer argument to a game entity.
         * The interface can pass the pointer as an opaque value to the
         * scripting engine, if needed. This value will usually be passed
         * by the script to some callback functions.
         */
        virtual void push(Entity *) = 0;

        /**
         * Pushes a list of items with amounts to the script engine.
         */
        virtual void push(const std::list<InventoryItem> &itemList) = 0;

        virtual void push(AttributeInfo *) = 0;

        /**
         * Executes the function being prepared.
         * @param context the context that is supposed to be used for executing
         * @return the value returned by the script.
         */
        virtual int execute(const Context &context = Context()) = 0;

        /**
         * Executes the function being prepared.
         * @param the map which is set as context
         * @return the value returned by the script.
         */
        int execute(MapComposite *map);

        /**
         * Starts or resumes the current thread. Deletes the thread when it is
         * done.
         *
         * @return whether the thread is done executing.
         */
        virtual bool resume() = 0;

        /**
         * Assigns the current callback to the given \a function.
         *
         * Where the callback exactly comes from is up to the script engine.
         */
        virtual void assignCallback(Ref &function) = 0;

        /**
         * Unreferences the script object given by \a ref, if any, and sets
         * \a ref to invalid.
         */
        virtual void unref(Ref &ref) = 0;

        /**
         * Returns the currently executing thread, or null when no thread is
         * currently executing.
         */
        Thread *getCurrentThread() const
        { return mCurrentThread; }

        /**
         * Returns the current context.
         */
        const Context *getContext() const
        { return mContext; }

        virtual void processDeathEvent(Entity *entity) = 0;

        virtual void processRemoveEvent(Entity *entity) = 0;

        static void setCreateNpcDelayedCallback(Script *script)
        { script->assignCallback(mCreateNpcDelayedCallback); }

        static void setUpdateCallback(Script *script)
        { script->assignCallback(mUpdateCallback); }

    protected:
        std::string mScriptFile;
        Thread *mCurrentThread;
        const Context *mContext;

    private:
        std::vector<Thread*> mThreads;

        static Ref mCreateNpcDelayedCallback;
        static Ref mUpdateCallback;

    friend struct ScriptEventDispatch;
    friend class Thread;
};

#endif // SCRIPTING_SCRIPT_H
