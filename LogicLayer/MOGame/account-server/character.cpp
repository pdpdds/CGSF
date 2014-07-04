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

#include "account-server/character.h"

#include "account-server/account.h"

#include "net/messagein.h"
#include "net/messageout.h"

CharacterData::CharacterData(const std::string &name, int id):
    mName(name),
    mDatabaseID(id),
    mCharacterSlot(0),
    mAccountID(-1),
    mAccount(nullptr),
    mMapId(0),
    mGender(0),
    mHairStyle(0),
    mHairColor(0),
    mAttributePoints(0),
    mCorrectionPoints(0),
    mAccountLevel(0)
{
}

void CharacterData::serialize(MessageOut &msg)
{
    // general character properties
    msg.writeInt8(getAccountLevel());
    msg.writeInt8(getGender());
    msg.writeInt8(getHairStyle());
    msg.writeInt8(getHairColor());
    msg.writeInt16(getAttributePoints());
    msg.writeInt16(getCorrectionPoints());


    const AttributeMap &attributes = getAttributes();
    msg.writeInt16(attributes.size());
    for (auto attributeIt : attributes)
    {
        msg.writeInt16(attributeIt.first);
        msg.writeDouble(attributeIt.second.getBase());
    }

    // status effects currently affecting the character
    msg.writeInt16(getStatusEffectSize());
    std::map<int, Status>::const_iterator status_it;
    for (status_it = getStatusEffectBegin(); status_it != getStatusEffectEnd(); status_it++)
    {
        msg.writeInt16(status_it->first);
        msg.writeInt16(status_it->second.time);
    }

    // location
    msg.writeInt16(getMapId());
    const Point &pos = getPosition();
    msg.writeInt16(pos.x);
    msg.writeInt16(pos.y);

    // kill count
    msg.writeInt16(getKillCountSize());
    std::map<int, int>::const_iterator kills_it;
    for (kills_it = getKillCountBegin(); kills_it != getKillCountEnd(); kills_it++)
    {
        msg.writeInt16(kills_it->first);
        msg.writeInt32(kills_it->second);
    }

    // character abilities
    const std::set<int> &abilities = getAbilities();
    msg.writeInt16(abilities.size());
    for (auto &abilityId : abilities) {
        msg.writeInt32(abilityId);
    }


    // questlog
    msg.writeInt16(mQuests.size());
    for (QuestInfo &quest : mQuests) {
        msg.writeInt16(quest.id);
        msg.writeInt8(quest.state);
        msg.writeString(quest.title);
        msg.writeString(quest.description);
    }

    // inventory - must be last because size isn't transmitted
    const Possessions &poss = getPossessions();
    const EquipData &equipData = poss.getEquipment();

    const InventoryData &inventoryData = poss.getInventory();
    for (InventoryData::const_iterator itemIt = inventoryData.begin(),
         itemIt_end = inventoryData.end(); itemIt != itemIt_end; ++itemIt)
    {
        int slot = itemIt->first;
        msg.writeInt16(slot);
        msg.writeInt16(itemIt->second.itemId);
        msg.writeInt16(itemIt->second.amount);
        if (equipData.find(itemIt->first) != equipData.end())
            msg.writeInt8(1); // equipped
        else
            msg.writeInt8(0); // not equipped
    }
}
void CharacterData::deserialize(MessageIn &msg)
{
    // general character properties
    setAccountLevel(msg.readInt8());
    setGender(ManaServ::getGender(msg.readInt8()));
    setHairStyle(msg.readInt8());
    setHairColor(msg.readInt8());
    setAttributePoints(msg.readInt16());
    setCorrectionPoints(msg.readInt16());

    // character attributes
    unsigned attrSize = msg.readInt16();
    for (unsigned i = 0; i < attrSize; ++i)
    {
        unsigned id = msg.readInt16();
        double base = msg.readDouble(),
               mod  = msg.readDouble();
        setAttribute(id, base);
        setModAttribute(id, mod);
    }

    // status effects currently affecting the character
    int statusSize = msg.readInt16();

    for (int i = 0; i < statusSize; i++)
    {
        int status = msg.readInt16();
        int time = msg.readInt16();
        applyStatusEffect(status, time);
    }

    // location
    setMapId(msg.readInt16());

    Point temporaryPoint;
    temporaryPoint.x = msg.readInt16();
    temporaryPoint.y = msg.readInt16();
    setPosition(temporaryPoint);

    // kill count
    int killSize = msg.readInt16();
    for (int i = 0; i < killSize; i++)
    {
        int monsterId = msg.readInt16();
        int kills = msg.readInt32();
        setKillCount(monsterId, kills);
    }

    // character abilities
    int abilitiesSize = msg.readInt16();
    clearAbilities();
    for (int i = 0; i < abilitiesSize; i++)
    {
        const int id = msg.readInt32();
        giveAbility(id);
    }

    // questlog
    int questlogSize = msg.readInt16();
    mQuests.clear();
    for (int i = 0; i < questlogSize; ++i) {
        QuestInfo quest;
        quest.id = msg.readInt16();
        quest.state = msg.readInt8();
        quest.title = msg.readString();
        quest.description = msg.readString();
        mQuests.push_back(quest);
    }

    // inventory - must be last because size isn't transmitted
    Possessions &poss = getPossessions();

    InventoryData inventoryData;
    EquipData equipmentData;
    while (msg.getUnreadLength())
    {
        InventoryItem i;
        i.slot          = msg.readInt16();
        i.itemId        = msg.readInt16();
        i.amount        = msg.readInt16();
        i.equipmentSlot = msg.readInt8();
        inventoryData.insert(std::make_pair(i.slot, i));
        if (i.equipmentSlot != 0)
            equipmentData.insert(i.slot);
    }
    poss.setInventory(inventoryData);
    poss.setEquipment(equipmentData);
}

void CharacterData::setAccount(Account *acc)
{
    mAccount = acc;
    mAccountID = acc->getID();
    mAccountLevel = acc->getLevel();
}

void CharacterData::giveAbility(int id)
{
    mAbilities.insert(id);
}
