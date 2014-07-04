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

#ifndef CHARACTERDATA_H
#define CHARACTERDATA_H

#include <string>
#include <vector>
#include <set>

#include "common/defines.h"
#include "common/inventorydata.h"
#include "utils/point.h"

class Account;
class MessageIn;
class MessageOut;

struct AttributeValue
{
    AttributeValue()
        : base(0)
        , modified(0)
    {}

    AttributeValue(double value)
        : base(value)
        , modified(value)
    {}

    double base;     /**< Base value of the attribute. */
    double modified; /**< Value after various modifiers have been applied. */

    double getBase() const
    { return base; }

    double getModifiedAttribute() const
    { return modified; }
};

struct Status
{
    Status()
        : time(0)
    {}

    unsigned time;
};

struct QuestInfo
{
    int id;
    int state;
    std::string title;
    std::string description;
};

/**
 * Stores attributes by their id.
 */
typedef std::map<unsigned, AttributeValue> AttributeMap;

class CharacterData
{
    public:
        CharacterData(const std::string &name, int id = -1);

        void serialize(MessageOut &msg);
        void deserialize(MessageIn &msg);

        /**
         * Gets the database id of the character.
         */
        int getDatabaseID() const { return mDatabaseID; }
        void setDatabaseID(int id) { mDatabaseID = id; }

        /**
         * Gets the slot of the character.
         */
        unsigned getCharacterSlot() const
        { return mCharacterSlot; }

        void setCharacterSlot(unsigned slot)
        { mCharacterSlot = slot; }

        /** Gets the account the character belongs to. */
        Account *getAccount() const
        { return mAccount; }

        /** Sets the account the character belongs to, and related fields. */
        void setAccount(Account *ptr);

        /**
         * Gets the ID of the account the character belongs to.
         */
        int getAccountID() const { return mAccountID; }
        void setAccountID(int id) { mAccountID = id; }

        /**
         * Gets the name of the character.
         */
        const std::string &getName() const { return mName; }
        void setName(const std::string &name) { mName = name; }

        /**
         * Gets the gender of the character (male / female).
         */
        int getGender() const { return mGender; }
        void setGender(int gender) { mGender = gender; }

        /**
         * Gets the hairstyle of the character.
         */
        int getHairStyle() const { return mHairStyle; }
        void setHairStyle(int style) { mHairStyle = style; }

        /**
         * Gets the haircolor of the character.
         */
        int getHairColor() const { return mHairColor; }
        void setHairColor(int color) { mHairColor = color; }

        /** Gets the account level of the user. */
        int getAccountLevel() const
        { return mAccountLevel; }

        /**
          * Sets the account level of the user.
          * @param force ensure the level is not modified by a game server.
          */
        void setAccountLevel(int l, bool force = false)
        { if (force) mAccountLevel = l; }

        /** Sets the value of a base attribute of the character. */
        void setAttribute(unsigned id, double value)
        { mAttributes[id].base = value; }

        void setModAttribute(unsigned id, double value)
        { mAttributes[id].modified = value; }

        const AttributeMap &getAttributes() const
        { return mAttributes; }

        /**
         * Get / Set a status effects
         */
        void applyStatusEffect(int id, int time)
        { mStatusEffects[id].time = time; }

        int getStatusEffectSize() const
        { return mStatusEffects.size(); }

        const std::map<int, Status>::const_iterator getStatusEffectBegin() const
        { return mStatusEffects.begin(); }

        const std::map<int, Status>::const_iterator getStatusEffectEnd() const
        { return mStatusEffects.end(); }

        /**
         * Get / Set kill count
         */
        int getKillCountSize() const
        { return mKillCount.size(); }

        const std::map<int, int>::const_iterator getKillCountBegin() const
        { return mKillCount.begin(); }

        const std::map<int, int>::const_iterator getKillCountEnd() const
        { return mKillCount.end(); }

        void setKillCount(int monsterId, int kills)
        { mKillCount[monsterId] = kills; }

        const std::set<int> &getAbilities() const
        { return mAbilities; }

        void clearAbilities()
        { mAbilities.clear(); }

        void giveAbility(int id);

        /**
         * Gets the Id of the map that the character is on.
         */
        int getMapId() const { return mMapId; }
        void setMapId(int mapId) { mMapId = mapId; }

        /**
         * Gets the position of the character on the map.
         */
        const Point &getPosition() const { return mPos; }
        void setPosition(const Point &p) { mPos = p; }

        /** Add a guild to the character */
        void addGuild(const std::string &name) { mGuilds.push_back(name); }

        /** Returns a list of guilds the player belongs to */
        std::vector<std::string>
        getGuilds() const { return mGuilds; }

        /**
         * Gets a reference on the possessions.
         */
        const Possessions &getPossessions() const
        { return mPossessions; }

        /**
         * Gets a reference on the possessions.
         */
        Possessions &getPossessions()
        { return mPossessions; }

        void setAttributePoints(int points)
        { mAttributePoints = points; }

        int getAttributePoints() const
        { return mAttributePoints; }

        void setCorrectionPoints(int points)
        { mCorrectionPoints = points; }

        int getCorrectionPoints() const
        { return mCorrectionPoints; }

    private:
        CharacterData(const CharacterData &) = delete;
        CharacterData &operator=(const CharacterData &) = delete;

        double getAttrBase(AttributeMap::const_iterator &it) const
        { return it->second.base; }
        double getAttrMod(AttributeMap::const_iterator &it) const
        { return it->second.modified; }

        Possessions mPossessions; //!< All the possesions of the character.
        std::string mName;        //!< Name of the character.
        int mDatabaseID;          //!< Character database ID.
        unsigned mCharacterSlot;  //!< Character slot.
        int mAccountID;           //!< Account ID of the owner.
        Account *mAccount;        //!< Account owning the character.
        Point mPos;               //!< Position the being is at.
        AttributeMap mAttributes; //!< Attributes.
        std::map<int, Status> mStatusEffects; //!< Status Effects
        std::map<int, int> mKillCount; //!< Kill Count
        std::set<int> mAbilities;
        unsigned short mMapId;    //!< Map the being is on.
        unsigned char mGender;    //!< Gender of the being.
        unsigned char mHairStyle; //!< Hair style of the being.
        unsigned char mHairColor; //!< Hair color of the being.
        short mAttributePoints;   //!< Unused character points.
        short mCorrectionPoints;  //!< Unused correction points.
        unsigned char mAccountLevel; //!< Level of the associated account.

        std::vector<std::string> mGuilds;        //!< All the guilds the player
                                                 //!< belongs to.
        std::vector<QuestInfo> mQuests;

        friend class AccountHandler;
        friend class Storage;
};

/**
 * Type definition for a list of Characters.
 */
typedef std::map<unsigned, CharacterData* > Characters;

#endif
