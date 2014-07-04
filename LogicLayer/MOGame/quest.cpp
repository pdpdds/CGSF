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

#include "quest.h"

#include "accountconnection.h"
#include "charactercomponent.h"
#include "utils/logger.h"

#include <cassert>
#include <list>
#include <map>
#include <string>

#include <sigc++/connection.h>

typedef std::list< QuestCallback * > QuestCallbacks;
typedef std::map< std::string, QuestCallbacks > PendingVariables;

struct PendingQuest
{
    Entity *character;
    sigc::connection removedConnection;
    sigc::connection disconnectedConnection;
    PendingVariables variables;
};

typedef std::map< int, PendingQuest > PendingQuests;

static PendingQuests pendingQuests;

bool getQuestVar(Entity *ch, const std::string &name, std::string &value)
{
    std::map< std::string, std::string >::iterator
        i = ch->getComponent<CharacterComponent>()->questCache.find(name);
    if (i == ch->getComponent<CharacterComponent>()->questCache.end())
        return false;
    value = i->second;
    return true;
}

void setQuestVar(Entity *ch, const std::string &name,
                 const std::string &value)
{
    auto *characterComponent =
            ch->getComponent<CharacterComponent>();

    std::map< std::string, std::string >::iterator
        i = characterComponent->questCache.lower_bound(name);
    if (i == characterComponent->questCache.end() || i->first != name)
    {
        characterComponent->questCache.insert(i, std::make_pair(name, value));
    }
    else if (i->second == value)
    {
        return;
    }
    else
    {
        i->second = value;
    }
    accountHandler->updateCharacterVar(ch, name, value);
}

void QuestRefCallback::triggerCallback(Entity *ch,
                                       const std::string &value) const
{
    if (!mRef.isValid())
        return;

    Script *s = ScriptManager::currentState();
    s->prepare(mRef);
    s->push(ch);
    s->push(mQuestName);
    s->push(value);
    s->execute(ch->getMap());
}

static void partialRemove(Entity *t)
{
    int id = t->getComponent<CharacterComponent>()->getDatabaseID();
    PendingVariables &variables = pendingQuests[id].variables;
    // Remove all the callbacks, but do not remove the variable names.
    for (PendingVariables::iterator i = variables.begin(),
         i_end = variables.end(); i != i_end; ++i)
    {
        i->second.clear();
    }
    // The listener is kept in case a fullRemove is needed later.
}

static void fullRemove(Entity &ch)
{
    int id = ch.getComponent<CharacterComponent>()->getDatabaseID();

    {
        PendingQuest &pendingQuest = pendingQuests[id];
        pendingQuest.removedConnection.disconnect();
        pendingQuest.disconnectedConnection.disconnect();
    }

    // Remove anything related to this character.
    pendingQuests.erase(id);
}

void recoverQuestVar(Entity *ch, const std::string &name,
                     QuestCallback *f)
{
    auto *characterComponent =
            ch->getComponent<CharacterComponent>();

    assert(characterComponent->questCache.find(name) ==
           characterComponent->questCache.end());
    int id = ch->getComponent<CharacterComponent>()->getDatabaseID();
    PendingQuests::iterator i = pendingQuests.lower_bound(id);
    if (i == pendingQuests.end() || i->first != id)
    {
        PendingQuest pendingQuest;
        pendingQuest.character = ch;

        /* Connect to removed and disconnected signals, because we cannot
         * afford to get invalid pointers, when we finally recover the
         * variable.
         */
        pendingQuest.removedConnection =
                ch->signal_removed.connect(sigc::ptr_fun(partialRemove));
        pendingQuest.disconnectedConnection =
                characterComponent->signal_disconnected.connect(
                        sigc::ptr_fun(fullRemove));

        i = pendingQuests.insert(i, std::make_pair(id, pendingQuest));
    }
    i->second.variables[name].push_back(f);
    accountHandler->requestCharacterVar(ch, name);
}

void recoveredQuestVar(int id,
                       const std::string &name,
                       const std::string &value)
{
    PendingQuests::iterator i = pendingQuests.find(id);
    if (i == pendingQuests.end())
        return;

    PendingQuest &pendingQuest = i->second;
    pendingQuest.removedConnection.disconnect();
    pendingQuest.disconnectedConnection.disconnect();

    PendingVariables &variables = pendingQuest.variables;
    PendingVariables::iterator j = variables.find(name);
    if (j == variables.end())
    {
        LOG_ERROR("Account server recovered an unexpected quest variable.");
        return;
    }

    Entity *ch = pendingQuest.character;
    auto *characterComponent = ch->getComponent<CharacterComponent>();
    characterComponent->questCache[name] = value;

    // Call the registered callbacks.
    for (QuestCallbacks::const_iterator k = j->second.begin(),
         k_end = j->second.end(); k != k_end; ++k)
    {
        (*k)->triggerCallback(ch, value);
        delete (*k);
    }

    variables.erase(j);
    if (variables.empty())
    {
        pendingQuests.erase(i);
    }
}
