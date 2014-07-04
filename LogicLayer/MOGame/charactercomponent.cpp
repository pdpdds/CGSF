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

#include "charactercomponent.h"

#include "common/configuration.h"
#include "accountconnection.h"
#include "attributemanager.h"
#include "buysell.h"
#include "inventory.h"
#include "item.h"
#include "itemmanager.h"
#include "gamehandler.h"
#include "map.h"
#include "mapcomposite.h"
#include "mapmanager.h"
#include "state.h"
#include "trade.h"
#include "scripting/scriptmanager.h"
#include "net/messagein.h"
#include "net/messageout.h"

#include "utils/logger.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits.h>

Script::Ref CharacterComponent::mDeathCallback;
Script::Ref CharacterComponent::mDeathAcceptedCallback;
Script::Ref CharacterComponent::mLoginCallback;

static bool executeCallback(Script::Ref function, Entity &entity)
{
    if (!function.isValid())
        return false;

    Script *script = ScriptManager::currentState();
    script->prepare(function);
    script->push(&entity);
    script->execute(entity.getMap());
    return true;
}


CharacterComponent::CharacterComponent(Entity &entity, MessageIn &msg):
    mClient(nullptr),
    mConnected(true),
    mTransactionHandler(nullptr),
    mDatabaseID(-1),
    mHairStyle(0),
    mHairColor(0),
    mSendAttributePointsStatus(false),
    mAttributePoints(0),
    mCorrectionPoints(0),
    mSendAbilityCooldown(false),
    mParty(0),
    mTransaction(TRANS_NONE),
    mTalkNpcId(0),
    mNpcThread(0),
    mBaseEntity(&entity)
{
    auto *beingComponent = entity.getComponent<BeingComponent>();

    auto &attributeScope = attributeManager->getAttributeScope(CharacterScope);
    LOG_DEBUG("Character creation: initialisation of "
              << attributeScope.size() << " attributes.");
    for (auto &attribute : attributeScope)
        beingComponent->createAttribute(attribute);

    auto *actorComponent = entity.getComponent<ActorComponent>();
    actorComponent->setWalkMask(Map::BLOCKMASK_WALL);
    actorComponent->setBlockType(BLOCKTYPE_CHARACTER);
    actorComponent->setSize(16);


    auto *abilityComponent = new AbilityComponent();
    entity.addComponent(abilityComponent);
    abilityComponent->signal_ability_changed.connect(
            sigc::mem_fun(this, &CharacterComponent::abilityStatusChanged));
    abilityComponent->signal_global_cooldown_activated.connect(
            sigc::mem_fun(this,
                          &CharacterComponent::abilityCooldownActivated));

    // Get character data.
    mDatabaseID = msg.readInt32();
    beingComponent->setName(msg.readString());

    deserialize(entity, msg);

    Inventory(&entity, mPossessions).initialize();

    beingComponent->signal_attribute_changed.connect(sigc::mem_fun(
            this, &CharacterComponent::attributeChanged));
}

CharacterComponent::~CharacterComponent()
{
    delete mNpcThread;
}

void CharacterComponent::deserialize(Entity &entity, MessageIn &msg)
{
    auto *beingComponent = entity.getComponent<BeingComponent>();

    // general character properties
    setAccountLevel(msg.readInt8());
    beingComponent->setGender(ManaServ::getGender(msg.readInt8()));
    setHairStyle(msg.readInt8());
    setHairColor(msg.readInt8());
    setAttributePoints(msg.readInt16());
    setCorrectionPoints(msg.readInt16());

    // character attributes
    unsigned attrSize = msg.readInt16();
    for (unsigned i = 0; i < attrSize; ++i)
    {
        unsigned id = msg.readInt16();
        double base = msg.readDouble();
        auto *attributeInfo = attributeManager->getAttributeInfo(id);
        if (attributeInfo)
            beingComponent->setAttribute(entity, attributeInfo, base);
    }

    // status effects currently affecting the character
    int statusSize = msg.readInt16();

    for (int i = 0; i < statusSize; i++)
    {
        int status = msg.readInt16();
        int time = msg.readInt16();
        beingComponent->applyStatusEffect(status, time);
    }

    // location
    auto *map = MapManager::getMap(msg.readInt16());
    entity.setMap(map);

    Point temporaryPoint;
    temporaryPoint.x = msg.readInt16();
    temporaryPoint.y = msg.readInt16();
    entity.getComponent<ActorComponent>()->setPosition(entity, temporaryPoint);

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
    for (int i = 0; i < abilitiesSize; i++)
    {
        const int id = msg.readInt32();
        entity.getComponent<AbilityComponent>()->giveAbility(id);
    }

    // questlog
    int questlogSize = msg.readInt16();
    for (int i = 0; i < questlogSize; ++i) {
        unsigned id = msg.readInt16();
        QuestState state = (QuestState) msg.readInt8();
        std::string title = msg.readString();
        std::string description = msg.readString();

        setQuestlog(id, state, title, description);
    }

    Possessions &poss = getPossessions();

    // Loads inventory - must be last because size isn't transmitted
    InventoryData inventoryData;
    EquipData equipmentData;
    while (msg.getUnreadLength())
    {
        InventoryItem i;
        i.slot          = msg.readInt16();
        i.itemId        = msg.readInt16();
        i.amount        = msg.readInt16();
        bool isEquipped = msg.readInt8() != 0;
        inventoryData.insert(std::make_pair(i.slot, i));
        if (isEquipped)
            equipmentData.insert(i.slot);
    }
    poss.setInventory(inventoryData);
    poss.setEquipment(equipmentData);
}

void CharacterComponent::serialize(Entity &entity, MessageOut &msg)
{
    auto *beingComponent = entity.getComponent<BeingComponent>();

    // general character properties
    msg.writeInt8(getAccountLevel());
    msg.writeInt8(beingComponent->getGender());
    msg.writeInt8(getHairStyle());
    msg.writeInt8(getHairColor());
    msg.writeInt16(getAttributePoints());
    msg.writeInt16(getCorrectionPoints());


    const AttributeMap &attributes = beingComponent->getAttributes();
    std::map<const AttributeInfo *, const Attribute *> attributesToSend;
    for (auto &attributeIt : attributes)
    {
        if (attributeIt.first->persistent)
            attributesToSend.insert(std::make_pair(attributeIt.first,
                                                   &attributeIt.second));
    }
    msg.writeInt16(attributesToSend.size());
    for (auto &attributeIt : attributesToSend)
    {
        msg.writeInt16(attributeIt.first->id);
        msg.writeDouble(attributeIt.second->getBase());
        msg.writeDouble(attributeIt.second->getModifiedAttribute());
    }

    // status effects currently affecting the character
    auto &statusEffects = beingComponent->getStatusEffects();
    msg.writeInt16(statusEffects.size());
    for (auto &statusIt : statusEffects)
    {
        msg.writeInt16(statusIt.first);
        msg.writeInt16(statusIt.second.time);
    }

    // location
    msg.writeInt16(entity.getMap()->getID());
    const Point &pos = entity.getComponent<ActorComponent>()->getPosition();
    msg.writeInt16(pos.x);
    msg.writeInt16(pos.y);

    // kill count
    msg.writeInt16(getKillCountSize());
    for (auto &killCountIt : mKillCount)
    {
        msg.writeInt16(killCountIt.first);
        msg.writeInt32(killCountIt.second);
    }

    // character abilities
    auto &abilities = entity.getComponent<AbilityComponent>()->getAbilities();
    msg.writeInt16(abilities.size());
    for (auto &abilityIt : abilities) {
        msg.writeInt32(abilityIt.first);
    }

    // questlog
    msg.writeInt16(mQuestlog.size());
    for (auto questlogIt : mQuestlog) {
        QuestInfo &quest = questlogIt.second;
        msg.writeInt16(quest.id);
        msg.writeInt8(quest.state);
        msg.writeString(quest.title);
        msg.writeString(quest.description);
    }

    // inventory - must be last because size isn't transmitted
    const Possessions &poss = getPossessions();

    const InventoryData &inventoryData = poss.getInventory();
    for (InventoryData::const_iterator itemIt = inventoryData.begin(),
         itemIt_end = inventoryData.end(); itemIt != itemIt_end; ++itemIt)
    {
        msg.writeInt16(itemIt->first);           // slot id
        msg.writeInt16(itemIt->second.itemId);
        msg.writeInt16(itemIt->second.amount);
        msg.writeInt8(itemIt->second.equipmentSlot);
    }
}

void CharacterComponent::characterDied(Entity *being)
{
    executeCallback(mDeathCallback, *being);
}

void CharacterComponent::respawn(Entity &entity)
{
    auto *beingComponent = entity.getComponent<BeingComponent>();

    if (beingComponent->getAction() != DEAD)
    {
        LOG_WARN("Character \"" << beingComponent->getName()
                 << "\" tried to respawn without being dead");
        return;
    }

    // Make it alive again
    beingComponent->setAction(entity, STAND);

    // Execute respawn callback when set
    if (executeCallback(mDeathAcceptedCallback, entity))
        return;

    // No script respawn callback set - fall back to hardcoded logic
    const double maxHp = beingComponent->getModifiedAttribute(
            attributeManager->getAttributeInfo(ATTR_MAX_HP));
    beingComponent->setAttribute(entity,
                                 attributeManager->getAttributeInfo(ATTR_HP),
                                 maxHp);
    // Warp back to spawn point.
    int spawnMap = Configuration::getValue("char_respawnMap", 1);
    int spawnX = Configuration::getValue("char_respawnX", 1024);
    int spawnY = Configuration::getValue("char_respawnY", 1024);

    GameState::enqueueWarp(&entity, MapManager::getMap(spawnMap),
                           Point(spawnX, spawnY));
}

void CharacterComponent::abilityStatusChanged(int id)
{
    mModifiedAbilities.insert(id);
}

void CharacterComponent::abilityCooldownActivated()
{
    mSendAbilityCooldown = true;
}

void CharacterComponent::sendAbilityUpdate(Entity &entity)
{
    auto &abilities = entity.getComponent<AbilityComponent>()->getAbilities();

    MessageOut msg(GPMSG_ABILITY_STATUS);
    for (unsigned id : mModifiedAbilities)
    {
        auto it = abilities.find(id);
        if (it == abilities.end())
            continue; // got deleted

        msg.writeInt8(id);
        msg.writeInt32(it->second.rechargeTimeout.remaining());
    }

    mModifiedAbilities.clear();
    gameHandler->sendTo(mClient, msg);
}

void CharacterComponent::sendAbilityCooldownUpdate(Entity &entity)
{
    MessageOut msg(GPMSG_ABILITY_COOLDOWN);
    auto *abilityComponent = entity.getComponent<AbilityComponent>();
    msg.writeInt16(abilityComponent->globalCooldown());
    gameHandler->sendTo(mClient, msg);
    mSendAbilityCooldown = false;
}

void CharacterComponent::sendAttributePointsStatus(Entity &entity)
{
    MessageOut msg(GPMSG_ATTRIBUTE_POINTS_STATUS);
    msg.writeInt16(mAttributePoints);
    msg.writeInt16(mCorrectionPoints);
    gameHandler->sendTo(mClient, msg);
    mSendAttributePointsStatus = false;
}

void CharacterComponent::sendQuestUpdate()
{
    MessageOut msg(GPMSG_QUESTLOG_STATUS);
    for (auto &questIt : mModifiedQuests) {
        const QuestInfo *quest = questIt.first;
        bool notify = questIt.second;
        msg.writeInt16(quest->id);
        int flags = QUESTLOG_UPDATE_STATE |
                QUESTLOG_UPDATE_TITLE |
                QUESTLOG_UPDATE_DESCRIPTION;
        if (notify)
            flags |= QUESTLOG_SHOW_NOTIFICATION;
        msg.writeInt8(flags);
        msg.writeInt8(quest->state);
        msg.writeString(quest->title);
        msg.writeString(quest->description);
    }
    mModifiedQuests.clear();
    gameHandler->sendTo(mClient, msg);
}

void CharacterComponent::markQuestAsModified(const QuestInfo *quest,
                                             bool sendNotification)
{
    const auto &it = mModifiedQuests.find(quest);
    if (it == mModifiedQuests.end()) {
        mModifiedQuests.insert(std::make_pair(quest, sendNotification));
        return;
    }
    it->second = sendNotification;
}
void CharacterComponent::markAllQuestsAsModified()
{
    for (auto &questIt : mQuestlog) {
        mModifiedQuests[&questIt.second] = false;
    }
}

void CharacterComponent::cancelTransaction()
{
    TransactionType t = mTransaction;
    mTransaction = TRANS_NONE;
    switch (t)
    {
        case TRANS_TRADE:
            static_cast< Trade * >(mTransactionHandler)->cancel();
            break;
        case TRANS_BUYSELL:
            static_cast< BuySell * >(mTransactionHandler)->cancel();
            break;
        case TRANS_NONE:
            return;
    }
}

Trade *CharacterComponent::getTrading() const
{
    return mTransaction == TRANS_TRADE
        ? static_cast< Trade * >(mTransactionHandler) : nullptr;
}

BuySell *CharacterComponent::getBuySell() const
{
    return mTransaction == TRANS_BUYSELL
        ? static_cast< BuySell * >(mTransactionHandler) : nullptr;
}

void CharacterComponent::setTrading(Trade *t)
{
    if (t)
    {
        cancelTransaction();
        mTransactionHandler = t;
        mTransaction = TRANS_TRADE;
    }
    else
    {
        assert(mTransaction == TRANS_NONE || mTransaction == TRANS_TRADE);
        mTransaction = TRANS_NONE;
    }
}

void CharacterComponent::setBuySell(BuySell *t)
{
    if (t)
    {
        cancelTransaction();
        mTransactionHandler = t;
        mTransaction = TRANS_BUYSELL;
    }
    else
    {
        assert(mTransaction == TRANS_NONE || mTransaction == TRANS_BUYSELL);
        mTransaction = TRANS_NONE;
    }
}

void CharacterComponent::sendStatus(Entity &entity)
{
    auto *beingComponent = entity.getComponent<BeingComponent>();
    MessageOut attribMsg(GPMSG_PLAYER_ATTRIBUTE_CHANGE);
    for (AttributeInfo *attribute : mModifiedAttributes)
    {
        attribMsg.writeInt16(attribute->id);
        attribMsg.writeInt32(beingComponent->getAttributeBase(attribute) * 256);
        attribMsg.writeInt32(beingComponent->getModifiedAttribute(attribute) * 256);
    }
    if (attribMsg.getLength() > 2)
        gameHandler->sendTo(mClient, attribMsg);
    mModifiedAttributes.clear();

    if (!mModifiedAbilities.empty())
        sendAbilityUpdate(entity);

    if (mSendAbilityCooldown)
        sendAbilityCooldownUpdate(entity);

    if (mSendAttributePointsStatus)
        sendAttributePointsStatus(entity);

    if (!mModifiedQuests.empty())
        sendQuestUpdate();
}

void CharacterComponent::modifiedAllAbilities(Entity &entity)
{
    for (auto &abilityIt : entity.getComponent<AbilityComponent>()->getAbilities())
        mModifiedAbilities.insert(abilityIt.first);
}

void CharacterComponent::modifiedAllAttributes(Entity &entity)
{
    auto *beingComponent = entity.getComponent<BeingComponent>();

    LOG_DEBUG("Marking all attributes as changed, requiring recalculation.");
    for (auto attribute : beingComponent->getAttributes())
    {
        beingComponent->recalculateBaseAttribute(entity, attribute.first);
        mModifiedAttributes.insert(attribute.first);
    }
}

void CharacterComponent::attributeChanged(Entity *entity,
                                          AttributeInfo *attribute)
{
    auto *beingComponent = entity->getComponent<BeingComponent>();

    // Inform the client of this attribute modification.
    accountHandler->updateAttributes(getDatabaseID(), attribute->id,
                                     beingComponent->getAttributeBase(attribute),
                                     beingComponent->getModifiedAttribute(attribute));
    mModifiedAttributes.insert(attribute);
}

void CharacterComponent::incrementKillCount(int monsterType)
{
    std::map<int, int>::iterator i = mKillCount.find(monsterType);
    if (i == mKillCount.end())
    {
        // Character has never murdered this species before
        mKillCount[monsterType] = 1;
    }
    else
    {
        // Character is a repeated offender
        mKillCount[monsterType] ++;
    }
}

int CharacterComponent::getKillCount(int monsterType) const
{
    std::map<int, int>::const_iterator i = mKillCount.find(monsterType);
    if (i != mKillCount.end())
        return i->second;
    return 0;
}

AttribmodResponseCode CharacterComponent::useCharacterPoint(Entity &entity,
                                                            AttributeInfo *attribute)
{
    auto *beingComponent = entity.getComponent<BeingComponent>();

    if (!attribute->modifiable)
        return ATTRIBMOD_INVALID_ATTRIBUTE;
    if (!mAttributePoints)
        return ATTRIBMOD_NO_POINTS_LEFT;

    setAttributePoints(mAttributePoints - 1);

    const double base = beingComponent->getAttributeBase(attribute);
    beingComponent->setAttribute(entity, attribute, base + 1);
    beingComponent->updateDerivedAttributes(entity, attribute);
    return ATTRIBMOD_OK;
}

AttribmodResponseCode CharacterComponent::useCorrectionPoint(Entity &entity,
                                                             AttributeInfo *attribute)
{
    auto *beingComponent = entity.getComponent<BeingComponent>();

    if (!attribute->modifiable)
        return ATTRIBMOD_INVALID_ATTRIBUTE;
    if (!mCorrectionPoints)
        return ATTRIBMOD_NO_POINTS_LEFT;
    if (beingComponent->getAttributeBase(attribute) <= 1)
        return ATTRIBMOD_DENIED;

    setCorrectionPoints(mCorrectionPoints - 1);
    setAttributePoints(mAttributePoints + 1);

    const double base = beingComponent->getAttributeBase(attribute);
    beingComponent->setAttribute(entity, attribute, base - 1);
    return ATTRIBMOD_OK;
}

void CharacterComponent::startNpcThread(Script::Thread *thread, int npcId)
{
    if (mNpcThread)
        delete mNpcThread;

    mNpcThread = thread;
    mTalkNpcId = npcId;

    resumeNpcThread();
}

void CharacterComponent::resumeNpcThread()
{
    Script *script = ScriptManager::currentState();

    assert(script->getCurrentThread() == mNpcThread);

    if (script->resume())
    {
        MessageOut msg(GPMSG_NPC_CLOSE);
        msg.writeInt16(mTalkNpcId);
        gameHandler->sendTo(mClient, msg);

        mTalkNpcId = 0;
        mNpcThread = 0;
    }
}

void CharacterComponent::disconnected(Entity &entity)
{
    mConnected = false;

    // Make the dead characters respawn, even in case of disconnection.
    if (entity.getComponent<BeingComponent>()->getAction() == DEAD)
        respawn(entity);
    else
        GameState::remove(&entity);

    signal_disconnected.emit(entity);
}
void CharacterComponent::triggerLoginCallback(Entity &entity)
{
    executeCallback(mLoginCallback, entity);
}

void CharacterComponent::markAllInfoAsChanged(Entity &entity)
{
    modifiedAllAbilities(entity);
    modifiedAllAttributes(entity);
    markAllQuestsAsModified();
}

void CharacterComponent::setQuestlog(unsigned id, QuestState state,
                                     const std::string &title,
                                     const std::string &description,
                                     bool sendNotification)
{
    auto &quest = mQuestlog[id];
    quest.id = id;
    quest.state = state;
    quest.title = title;
    quest.description = description;

    markQuestAsModified(&quest, sendNotification);
}

void CharacterComponent::setQuestlogState(unsigned id,
                                          QuestState state,
                                          bool sendNotification)
{
    auto &quest = mQuestlog[id];
    quest.id = id;
    quest.state = state;

    markQuestAsModified(&quest, sendNotification);
}

void CharacterComponent::setQuestlogTitle(unsigned id,
                                          const std::string &title,
                                          bool sendNotification)
{
    auto &quest = mQuestlog[id];
    quest.id = id;
    quest.title = title;

    markQuestAsModified(&quest, sendNotification);
}

void CharacterComponent::setQuestlogDescription(unsigned id,
                                                const std::string &description,
                                                bool sendNotification)
{
    auto &quest = mQuestlog[id];
    quest.id = id;
    quest.description = description;

    markQuestAsModified(&quest, sendNotification);
}
