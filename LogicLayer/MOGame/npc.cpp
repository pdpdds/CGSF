/*
 *  The Mana Server
 *  Copyright (C) 2007-2010  The Mana World Development Team
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

#include "npc.h"

#include "charactercomponent.h"
#include "gamehandler.h"
#include "map.h"
#include "net/messageout.h"
#include "scripting/script.h"
#include "scripting/scriptmanager.h"

NpcComponent::NpcComponent(int npcId):
    mNpcId(npcId),
    mEnabled(true)
{
}

NpcComponent::~NpcComponent()
{
    Script *script = ScriptManager::currentState();
    script->unref(mTalkCallback);
    script->unref(mUpdateCallback);
}

void NpcComponent::setEnabled(bool enabled)
{
    mEnabled = enabled;
}

void NpcComponent::update(Entity &entity)
{
    if (!mEnabled || !mUpdateCallback.isValid())
        return;

    Script *script = ScriptManager::currentState();
    script->prepare(mUpdateCallback);
    script->push(&entity);
    script->execute(entity.getMap());
}

void NpcComponent::setTalkCallback(Script::Ref function)
{
    ScriptManager::currentState()->unref(mTalkCallback);
    mTalkCallback = function;
}

void NpcComponent::setUpdateCallback(Script::Ref function)
{
    ScriptManager::currentState()->unref(mUpdateCallback);
    mUpdateCallback = function;
}



static Script *prepareResume(Entity *ch, Script::ThreadState expectedState)
{
    Script::Thread *thread =
            ch->getComponent<CharacterComponent>()->getNpcThread();
    if (!thread || thread->mState != expectedState)
        return 0;

    Script *script = ScriptManager::currentState();
    script->prepareResume(thread);
    return script;
}

void Npc::start(Entity *npc, Entity *ch)
{
    NpcComponent *npcComponent = npc->getComponent<NpcComponent>();

    Script *script = ScriptManager::currentState();
    Script::Ref talkCallback = npcComponent->getTalkCallback();

    if (npcComponent->isEnabled() && talkCallback.isValid())
    {
        Script::Thread *thread = script->newThread();
        thread->getContext().map = npc->getMap();
        thread->getContext().npc = npc;
        thread->getContext().character = ch;
        script->prepare(talkCallback);
        script->push(npc);
        script->push(ch);
        auto *actorComponent = npc->getComponent<ActorComponent>();
        ch->getComponent<CharacterComponent>()->startNpcThread(
                thread, actorComponent->getPublicID());
    }
}

void Npc::resume(Entity *ch)
{
    if (prepareResume(ch, Script::ThreadPaused))
        ch->getComponent<CharacterComponent>()->resumeNpcThread();
}

void Npc::integerReceived(Entity *ch, int value)
{
    if (Script *script = prepareResume(ch, Script::ThreadExpectingNumber))
    {
        script->push(value);
        ch->getComponent<CharacterComponent>()->resumeNpcThread();
    }
}

void Npc::stringReceived(Entity *ch, const std::string &value)
{
    if (Script *script = prepareResume(ch, Script::ThreadExpectingString))
    {
        script->push(value);
        ch->getComponent<CharacterComponent>()->resumeNpcThread();
    }
}
