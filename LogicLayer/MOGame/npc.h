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

#ifndef GAMESERVER_NPC_H
#define GAMESERVER_NPC_H

#include "component.h"
#include "scripting/script.h"

class CharacterComponent;

/**
 * Component describing a non-player character.
 */
class NpcComponent : public Component
{
    public:
        static const ComponentType type = CT_Npc;

        NpcComponent(int npcId);

        ~NpcComponent();

        /**
         * Sets the function that should be called when this NPC is talked to.
         */
        void setTalkCallback(Script::Ref function);

        Script::Ref getTalkCallback() const
        { return mTalkCallback; }

        /**
         * Sets the function that should be called each update.
         */
        void setUpdateCallback(Script::Ref function);

        /**
         * Calls the update callback, if any.
         */
        void update(Entity &entity);

        /**
         * Sets whether the NPC is enabled.
         *
         * When disabling an NPC, it does currently not cancel already started
         * conversations with this NPC!
         */
        void setEnabled(bool enabled);

        bool isEnabled() const
        { return mEnabled; }

        /**
         * Gets NPC ID.
         */
        int getNpcId() const
        { return mNpcId; }

    private:
        int mNpcId;
        bool mEnabled;

        Script::Ref mTalkCallback;
        Script::Ref mUpdateCallback;
};


namespace Npc {

/**
 * Starts a conversation with the NPC.
 */
void start(Entity *npc, Entity *ch);

/**
 * Resumes an NPC conversation.
 */
void resume(Entity *ch);

/**
 * The player has made a choice or entered an integer.
 */
void integerReceived(Entity *ch, int value);

/**
 * The player has entered an string.
 */
void stringReceived(Entity *ch, const std::string &value);

} // namespace Npc


#endif // GAMESERVER_NPC_H
