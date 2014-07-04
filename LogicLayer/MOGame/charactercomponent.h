/*
 *  The Mana Server
 *  Copyright (C) 2004-2010  The Mana World Development Team
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

#ifndef CHARACTER_H
#define CHARACTER_H

#include "common/defines.h"
#include "common/inventorydata.h"
#include "common/manaserv_protocol.h"

#include "abilitycomponent.h"
#include "being.h"
#include "mapcomposite.h"
#include "mapmanager.h"
#include "abilitymanager.h"

#include "scripting/script.h"

#include "utils/logger.h"

#include <map>
#include <set>
#include <string>
#include <vector>

class BuySell;
struct GameClient;
class MessageIn;
class MessageOut;
class Point;
class Trade;

enum QuestState
{
    QUEST_OPEN = 0,
    QUEST_FINISHED,
    QUEST_FAILED,
};

struct QuestInfo
{
    QuestInfo()
        : id(0)
        , state(QUEST_OPEN)
    {}

    unsigned id;
    QuestState state;
    std::string title;
    std::string description;
};

/**
 * The representation of a player's character in the game world.
 */
class CharacterComponent : public Component
{
    public:
        static const ComponentType type = CT_Character;

        /**
         * Utility constructor for creating a Character from a received
         * characterdata message.
         */
        CharacterComponent(Entity &entity, MessageIn &msg);

        ~CharacterComponent();

        virtual void update(Entity &entity);

        /**
         * Executes the global die script
         */
        virtual void characterDied(Entity *);

        /**
         * makes the character respawn
         */
        void respawn(Entity &entity);

        /**
         * Gets client computer.
         */
        GameClient *getClient() const
        { return mClient; }

        /**
         * Sets client computer.
         */
        void setClient(GameClient *c)
        { mClient = c; }

        /**
         * Gets a reference to the possessions.
         */
        const Possessions &getPossessions() const
        { return mPossessions; }

        /**
         * Gets a reference to the possessions.
         */
        Possessions &getPossessions()
        { return mPossessions; }

        /**
         * Gets the Trade object the character is involved in.
         */
        Trade *getTrading() const;

        /**
         * Sets the Trade object the character is involved in.
         * Cancels other transactions.
         */
        void setTrading(Trade *t);

        /**
         * Gets the BuySell object the character is involved in.
         */
        BuySell *getBuySell() const;

        /**
         * Sets the trade object the character is involved in.
         * Cancels other transactions.
         */
        void setBuySell(BuySell *t);

        /**
         * Cancels current transaction.
         */
        void cancelTransaction();

        /**
         * Gets transaction status of the character.
         */
        bool isBusy() const
        { return mTransaction != TRANS_NONE; }

        /*
         * Character data:
         * Get and set methods
         * Most of this should be accessed directly as a friend
         */

        int getDatabaseID() const { return mDatabaseID; }
        void setDatabaseID(int id) { mDatabaseID = id; }

        int getHairStyle() const { return mHairStyle; }
        void setHairStyle(int style) { mHairStyle = style; }

        int getHairColor() const { return mHairColor; }
        void setHairColor(int color) { mHairColor = color; }

        int getAccountLevel() const { return mAccountLevel; }
        void setAccountLevel(int l) { mAccountLevel = l; }

        /** Gets the party id of the character */
        int getParty() const
        { return mParty; }

        /** Sets the party id of the character */
        void setParty(int party)
        { mParty = party; }

        /**
         * Sends a message that informs the client about attribute
         * modified since last call.
         */
        void sendStatus(Entity &entity);

        void modifiedAllAbilities(Entity &entity);
        void modifiedAllAttributes(Entity &entity);

        /**
         * Signal handler for attribute changed event
         * Flags the attribute as modified.
         * @param being th being of which the attribute was changed
         * @param attributeId the changed id
         */
        void attributeChanged(Entity *being, AttributeInfo *);

        /**
         * Calls all the "disconnected" listener.
         */
        void disconnected(Entity &entity);

        /**
         * Associative array containing all the quest variables known by the
         * server.
         */
        std::map< std::string, std::string > questCache;

        /**
         * Used to serialize kill count.
         */
        int getKillCountSize() const
        { return mKillCount.size(); }

        const std::map<int, int>::const_iterator getKillCountBegin() const
        { return mKillCount.begin(); }

        const std::map<int, int>::const_iterator getKillCountEnd() const
        { return mKillCount.end(); }

        void setKillCount(int monsterId, int kills)
        { mKillCount[monsterId] = kills; }

        /**
         * Adds one kill of the monster type to the characters kill count.
         */
        void incrementKillCount(int monsterType);

        /**
         * Gets the number of monsters the character killed of a given type.
         */
        int getKillCount(int monsterType) const;

        /**
         * Tries to use a character point to increase a
         * basic attribute
         */
        AttribmodResponseCode useCharacterPoint(Entity &entity,
                                                AttributeInfo *);

        /**
         * Tries to use a correction point to reduce a
         * basic attribute and regain a character point
         */
        AttribmodResponseCode useCorrectionPoint(Entity &entity,
                                                 AttributeInfo *);

        void setAttributePoints(int points);
        int getAttributePoints() const;

        void setCorrectionPoints(int points);
        int getCorrectionPoints() const;


        /**
         * Starts the given NPC thread.
         *
         * Should be called immediately after creating the thread and pushing
         * the NPC function and its parameters.
         */
        void startNpcThread(Script::Thread *thread, int npcId);

        /**
         * Resumes the given NPC thread of this character and sends the NPC
         * close message to the player when the script is done.
         *
         * Should be called after preparing the current Script instance for
         * resuming the thread and pushing the parameters the script expects.
         */
        void resumeNpcThread();

        /**
         * Returns the NPC thread in use by this character, if any.
         */
        Script::Thread *getNpcThread() const
        { return mNpcThread; }

        /** Makes it impossible to chat for a while */
        void mute(int seconds)
        { mMuteTimeout.set(seconds * 10); }

        bool isMuted() const
        { return !mMuteTimeout.expired(); }

        bool isConnected() const
        { return mConnected; }

        static void setDeathCallback(Script *script)
        { script->assignCallback(mDeathCallback); }

        static void setDeathAcceptedCallback(Script *script)
        { script->assignCallback(mDeathAcceptedCallback); }

        static void setLoginCallback(Script *script)
        { script->assignCallback(mLoginCallback); }

        void triggerLoginCallback(Entity &entity);

        void markAllInfoAsChanged(Entity &entity);

        void setQuestlog(unsigned id, QuestState state,
                         const std::string &title,
                         const std::string &description,
                         bool sendNotification = false);
        void setQuestlogState(unsigned id, QuestState state,
                              bool sendNotification = false);
        void setQuestlogTitle(unsigned id,
                              const std::string &title,
                              bool sendNotification = false);
        void setQuestlogDescription(unsigned id,
                                    const std::string &description,
                                    bool sendNotification = false);

        sigc::signal<void, Entity &> signal_disconnected;

        void serialize(Entity &entity, MessageOut &msg);

    private:
        void deserialize(Entity &entity, MessageIn &msg);

        void abilityStatusChanged(int id);
        void abilityCooldownActivated();

        void sendAbilityUpdate(Entity &entity);
        void sendAbilityCooldownUpdate(Entity &entity);
        void sendAttributePointsStatus(Entity &entity);
        void sendQuestUpdate();

        void markQuestAsModified(const QuestInfo *quest, bool sendNotification);
        void markAllQuestsAsModified();

        enum TransactionType
        { TRANS_NONE, TRANS_TRADE, TRANS_BUYSELL };

        GameClient *mClient;   /**< Client computer. */

        /**
         * Tells whether the character client is connected.
         * Useful when dealing with enqueued events.
         */
        bool mConnected;

        /** Handler of the transaction the character is involved in. */
        void *mTransactionHandler;

        Possessions mPossessions;    /**< Possesssions of the character. */

        /** Attributes modified since last update. */
        std::set<AttributeInfo *> mModifiedAttributes;
        std::set<unsigned> mModifiedAbilities;
        std::map<const QuestInfo *, bool> mModifiedQuests;

        int mDatabaseID;             /**< Character's database ID. */
        unsigned char mHairStyle;    /**< Hair Style of the character. */
        unsigned char mHairColor;    /**< Hair Color of the character. */

        bool mSendAttributePointsStatus;
        int mAttributePoints;        /**< Unused attribute points that can be distributed */
        int mCorrectionPoints;       /**< Unused attribute correction points */

        bool mSendAbilityCooldown;
        unsigned char mAccountLevel; /**< Account level of the user. */
        int mParty;                  /**< Party id of the character */
        TransactionType mTransaction; /**< Trade/buy/sell action the character is involved in. */
        std::map<int, int> mKillCount;  /**< How many monsters the character has slain of each type */

        int mTalkNpcId;              /**< Public ID of NPC the character is talking to, if any */
        Script::Thread *mNpcThread;  /**< Script thread executing NPC interaction, if any */

        Timeout mMuteTimeout;        /**< Time until the character is no longer muted  */

        std::map<unsigned, QuestInfo> mQuestlog;

        Entity *mBaseEntity;        /**< The entity this component is part of
                                         this is ONLY required to allow using
                                         the serialization routine without many
                                         changes (we cannot pass the entity as
                                         argument there). DO NOT USE THIS IF IT
                                         IS AVOIDABLE in order to allow
                                         refactoring this easier later! */

        static Script::Ref mDeathCallback;
        static Script::Ref mDeathAcceptedCallback;
        static Script::Ref mLoginCallback;
};


inline void CharacterComponent::setAttributePoints(int points)
{
    mSendAttributePointsStatus = true;
    mAttributePoints = points;
}

inline int CharacterComponent::getAttributePoints() const
{
    return mAttributePoints;
}

inline void CharacterComponent::setCorrectionPoints(int points)
{
    mSendAttributePointsStatus = true;
    mCorrectionPoints = points;
}

inline int CharacterComponent::getCorrectionPoints() const
{
    return mCorrectionPoints;
}

inline void CharacterComponent::update(Entity &entity)
{
}

#endif // CHARACTER_H
