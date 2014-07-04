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

#include <cassert>
#include <time.h>

#include "account-server/storage.h"

#include "account-server/account.h"
#include "account-server/character.h"
#include "account-server/flooritem.h"
#include "chat-server/chatchannel.h"
#include "chat-server/guild.h"
#include "chat-server/post.h"
#include "common/configuration.h"
#include "common/manaserv_protocol.h"
#include "dal/dalexcept.h"
#include "dal/dataproviderfactory.h"
#include "utils/functors.h"
#include "utils/point.h"
#include "utils/string.h"
#include "utils/throwerror.h"
#include "utils/xml.h"

#include <stdint.h>

static const char *DEFAULT_ITEM_FILE = "items.xml";

// Defines the supported db version
static const char *DB_VERSION_PARAMETER = "database_version";

/*
 * MySQL specificities:
 *     - TINYINT is an integer (1 byte) type defined as an extension to
 *       the SQL standard.
 *     - all integer types can have an optional (non-standard) attribute
 *       UNSIGNED (http://dev.mysql.com/doc/mysql/en/numeric-types.html)
 *
 * SQLite3 specificities:
 *     - any column (but only one for each table) with the exact type of
 *       'INTEGER PRIMARY KEY' is taken as auto-increment.
 *     - the supported data types are: NULL, INTEGER, REAL, TEXT and BLOB
 *       (http://www.sqlite.org/datatype3.html)
 *     - the size of TEXT cannot be set, it is just ignored by the engine.
 *     - IMPORTANT: foreign key constraints are not yet supported
 *       (http://www.sqlite.org/omitted.html). Included in case of future
 *       support.
 *
 * Notes:
 *     - the SQL queries will take advantage of the most appropriate data
 *       types supported by a particular database engine in order to
 *       optimize the server database size.
 *
 * TODO: Fix problem with PostgreSQL null primary key's.
 */

// Table names
static const char *ACCOUNTS_TBL_NAME            =   "mana_accounts";
static const char *CHARACTERS_TBL_NAME          =   "mana_characters";
static const char *CHAR_ATTR_TBL_NAME           =   "mana_char_attr";
static const char *CHAR_STATUS_EFFECTS_TBL_NAME =   "mana_char_status_effects";
static const char *CHAR_KILL_COUNT_TBL_NAME     =   "mana_char_kill_stats";
static const char *CHAR_ABILITIES_TBL_NAME      =   "mana_char_abilities";
static const char *QUESTLOG_TBL_NAME            =   "mana_questlog";
static const char *INVENTORIES_TBL_NAME         =   "mana_inventories";
static const char *ITEMS_TBL_NAME               =   "mana_items";
static const char *GUILDS_TBL_NAME              =   "mana_guilds";
static const char *GUILD_MEMBERS_TBL_NAME       =   "mana_guild_members";
static const char *QUESTS_TBL_NAME              =   "mana_quests";
static const char *WORLD_STATES_TBL_NAME        =   "mana_world_states";
static const char *POST_TBL_NAME                =   "mana_post";
static const char *POST_ATTACHMENTS_TBL_NAME    =   "mana_post_attachments";
static const char *AUCTION_TBL_NAME             =   "mana_auctions";
static const char *AUCTION_BIDS_TBL_NAME        =   "mana_auction_bids";
static const char *ONLINE_USERS_TBL_NAME        =   "mana_online_list";
static const char *TRANSACTION_TBL_NAME         =   "mana_transactions";
static const char *FLOOR_ITEMS_TBL_NAME         =   "mana_floor_items";

Storage::Storage()
        : mDb(dal::DataProviderFactory::createDataProvider()),
          mItemDbVersion(0)
{
}

Storage::~Storage()
{
    if (mDb->isConnected())
        close();

    delete mDb;
}

void Storage::open()
{
    // Do nothing if already connected.
    if (mDb->isConnected())
        return;

    using namespace dal;

    try
    {
        // Open a connection to the database.
        mDb->connect();

        // Check database version here
        int dbversion = utils::stringToInt(
                    getWorldStateVar(DB_VERSION_PARAMETER, SystemMap));
        int supportedDbVersion = ManaServ::SUPPORTED_DB_VERSION;
//20140621
        /*if (dbversion != supportedDbVersion)
        {
            std::ostringstream errmsg;
            errmsg << "Database version is not supported. "
                   << "Needed version: '" << supportedDbVersion
                   << "', current version: '" << dbversion << "'";
            utils::throwError(errmsg.str());
        }*/

        // Synchronize base data from xml files
        syncDatabase();

        // Clean list of online users, this should be empty after restart
        std::ostringstream sql;
        sql << "DELETE FROM " << ONLINE_USERS_TBL_NAME;
        mDb->execSql(sql.str());

        // In case where the server shouldn't keep floor item in database,
        // we remove remnants at startup
        if (Configuration::getValue("game_floorItemDecayTime", 0) > 0)
        {
            sql.clear();
            sql << "DELETE FROM " << FLOOR_ITEMS_TBL_NAME;
            mDb->execSql(sql.str());
        }
    }
    catch (const DbConnectionFailure& e)
    {
        utils::throwError("(DALStorage::open) "
                          "Unable to connect to the database: ", e);
    }
}

void Storage::close()
{
    mDb->disconnect();
}

Account *Storage::getAccountBySQL()
{
    try
    {
        const dal::RecordSet &accountInfo = mDb->processSql();

        // If the account is not even in the database then
        // we have no choice but to return nothing.
        if (accountInfo.isEmpty())
            return 0;

        string_to< unsigned > toUint;
        unsigned id = toUint(accountInfo(0, 0));

        // Create an Account instance
        // and initialize it with information about the user.
        Account *account = new Account(id);
        account->setName(accountInfo(0, 1));
        account->setPassword(accountInfo(0, 2));
        account->setEmail(accountInfo(0, 3));
        account->setRegistrationDate(toUint(accountInfo(0, 6)));
        account->setLastLogin(toUint(accountInfo(0, 7)));

        int level = toUint(accountInfo(0, 4));
        // Check if the user is permanently banned, or temporarily banned.
        if (level == AL_BANNED
            || time(0) <= (int) toUint(accountInfo(0, 5)))
        {
            account->setLevel(AL_BANNED);
            // It is, so skip character loading.
            return account;
        }
        account->setLevel(level);

        // Correct on-the-fly the old 0 slot characters
        // NOTE: Will be deprecated and removed at some point.
        fixCharactersSlot(id);

        // Load the characters associated with the account.
        std::ostringstream sql;
        sql << "select id from " << CHARACTERS_TBL_NAME << " where user_id = '"
            << id << "';";
        const dal::RecordSet &charInfo = mDb->execSql(sql.str());

        if (!charInfo.isEmpty())
        {
            int size = charInfo.rows();
            Characters characters;

            LOG_DEBUG("Account "<< id << " has " << size
                      << " character(s) in database.");

            // Two steps: it seems like multiple requests cannot be alive
            // at the same time.
            std::vector< unsigned > characterIDs;
            for (int k = 0; k < size; ++k)
                characterIDs.push_back(toUint(charInfo(k, 0)));

            for (int k = 0; k < size; ++k)
            {
                if (CharacterData *ptr =
                        getCharacter(characterIDs[k], account))
                {
                    characters[ptr->getCharacterSlot()] = ptr;
                }
                else
                {
                    LOG_ERROR("Failed to get character " << characterIDs[k]
                              << " for account " << id << '.');
                }
            }

            account->setCharacters(characters);
        }

        return account;
    }
    catch (const dal::DbSqlQueryExecFailure &e)
    {
        utils::throwError("(DALStorage::getAccountBySQL) SQL query failure: ",
                          e);
    }

    return 0;
}

void Storage::fixCharactersSlot(int accountId)
{
    try
    {
        // Obtain all the characters slots from an account.
        std::ostringstream sql;
        sql << "SELECT id, slot FROM " << CHARACTERS_TBL_NAME
            << " where user_id = " << accountId;
        const dal::RecordSet &charInfo = mDb->execSql(sql.str());

        // If the account is not even in the database then
        // we can quit now.
        if (charInfo.isEmpty())
            return;

        string_to< unsigned > toUint;
        std::map<unsigned, unsigned> slotsToUpdate;

        int characterNumber = charInfo.rows();
        unsigned currentSlot = 1;

        // We parse all the characters slots to see how many are to be
        // corrected.
        for (int k = 0; k < characterNumber; ++k)
        {
            // If the slot found is equal to 0.
            if (toUint(charInfo(k, 1)) == 0)
            {
                // Find the new slot number to assign.
                for (int l = 0; l < characterNumber; ++l)
                {
                    if (toUint(charInfo(l, 1)) == currentSlot)
                        currentSlot++;
                }
                slotsToUpdate.insert(std::make_pair(toUint(charInfo(k, 0)),
                                                    currentSlot));
            }
        }

        if (!slotsToUpdate.empty())
        {
            dal::PerformTransaction transaction(mDb);

            // Update the slots in database.
            for (std::map<unsigned, unsigned>::iterator i =
                                                          slotsToUpdate.begin(),
                i_end = slotsToUpdate.end(); i != i_end; ++i)
            {
                // Update the character slot.
                sql.clear();
                sql.str("");
                sql << "UPDATE " << CHARACTERS_TBL_NAME
                    << " SET slot = " << i->second
                    << " where id = " << i->first;
                mDb->execSql(sql.str());
            }

            transaction.commit();
        }
    }
    catch (const dal::DbSqlQueryExecFailure &e)
    {
        utils::throwError("(DALStorage::fixCharactersSlots) "
                          "SQL query failure: ", e);
    }
}

Account *Storage::getAccount(const std::string &userName)
{
    std::ostringstream sql;
    sql << "SELECT * FROM " << ACCOUNTS_TBL_NAME << " WHERE username = ?";
    if (mDb->prepareSql(sql.str()))
    {
        mDb->bindValue(1, userName);
        return getAccountBySQL();
    }
    return 0;
}

Account *Storage::getAccount(int accountID)
{
    std::ostringstream sql;
    sql << "SELECT * FROM " << ACCOUNTS_TBL_NAME << " WHERE id = ?";
    if (mDb->prepareSql(sql.str()))
    {
        mDb->bindValue(1, accountID);
        return getAccountBySQL();
    }
    return 0;
}

CharacterData *Storage::getCharacterBySQL(Account *owner)
{
    CharacterData *character = 0;

    string_to< unsigned > toUint;
    string_to< int > toInt;

    try
    {
        const dal::RecordSet &charInfo = mDb->processSql();

        // If the character is not even in the database then
        // we have no choice but to return nothing.
        if (charInfo.isEmpty())
            return 0;

        string_to< unsigned short > toUshort;
        string_to< double > toDouble;

        character = new CharacterData(charInfo(0, 2), toUint(charInfo(0, 0)));
        character->setGender(toUshort(charInfo(0, 3)));
        character->setHairStyle(toUshort(charInfo(0, 4)));
        character->setHairColor(toUshort(charInfo(0, 5)));
        character->setAttributePoints(toUshort(charInfo(0, 6)));
        character->setCorrectionPoints(toUshort(charInfo(0, 7)));
        Point pos(toInt(charInfo(0, 8)), toInt(charInfo(0, 9)));
        character->setPosition(pos);

        int mapId = toUint(charInfo(0, 10));
        if (mapId > 0)
        {
            character->setMapId(mapId);
        }
        else
        {
            // Set character to default map and one of the default location
            // Default map is to be 1, as not found return value will be 0.
            character->setMapId(Configuration::getValue("char_defaultMap", 1));
        }

        character->setCharacterSlot(toUint(charInfo(0, 11)));

        // Fill the account-related fields. Last step, as it may require a new
        // SQL query.
        if (owner)
        {
            character->setAccount(owner);
        }
        else
        {
            int id = toUint(charInfo(0, 1));
            character->setAccountID(id);
            std::ostringstream s;
            s << "select level from " << ACCOUNTS_TBL_NAME
              << " where id = '" << id << "';";
            const dal::RecordSet &levelInfo = mDb->execSql(s.str());
            character->setAccountLevel(toUint(levelInfo(0, 0)), true);
        }

        std::ostringstream s;

        // Load attributes.
        {
            s << "SELECT attr_id, attr_base, attr_mod "
              << "FROM " << CHAR_ATTR_TBL_NAME << " "
              << "WHERE char_id = " << character->getDatabaseID();

            const dal::RecordSet &attrInfo = mDb->execSql(s.str());
            const unsigned nRows = attrInfo.rows();
            for (unsigned row = 0; row < nRows; ++row)
            {
                unsigned id = toUint(attrInfo(row, 0));
                character->setAttribute(id,    toDouble(attrInfo(row, 1)));
                character->setModAttribute(id, toDouble(attrInfo(row, 2)));
            }
        }

        // Load the status effects
        {
            s.clear();
            s.str("");
            s << "select status_id, status_time FROM "
              << CHAR_STATUS_EFFECTS_TBL_NAME
              << " WHERE char_id = " << character->getDatabaseID();
            const dal::RecordSet &statusInfo = mDb->execSql(s.str());
            const unsigned nRows = statusInfo.rows();
            for (unsigned row = 0; row < nRows; row++)
            {
                character->applyStatusEffect(
                    toUint(statusInfo(row, 0)), // Status Id
                    toUint(statusInfo(row, 1))); // Time
            }
        }

        // Load the kill stats
        {
            s.clear();
            s.str("");
            s << "select monster_id, kills FROM " << CHAR_KILL_COUNT_TBL_NAME
              << " WHERE char_id = " << character->getDatabaseID();
            const dal::RecordSet &killsInfo = mDb->execSql(s.str());
            const unsigned nRows = killsInfo.rows();
            for (unsigned row = 0; row < nRows; row++)
            {
                character->setKillCount(
                    toUint(killsInfo(row, 0)), // MonsterID
                    toUint(killsInfo(row, 1))); // Kills
            }
        }

        // Load the ability status
        {
            s.clear();
            s.str("");
            s << "SELECT ability_id FROM "
              << CHAR_ABILITIES_TBL_NAME
              << " WHERE char_id = " << character->getDatabaseID();
            const dal::RecordSet &abilitiesInfo = mDb->execSql(s.str());
            const unsigned nRows = abilitiesInfo.rows();
            for (unsigned row = 0; row < nRows; row++)
            {
                character->giveAbility(toUint(abilitiesInfo(row, 0)));
            }
        }

        // Load the questlog
        {
            s.clear();
            s.str("");
            s << "SELECT quest_id, quest_state, quest_title, quest_description "
              << "FROM " << QUESTLOG_TBL_NAME
              << " WHERE char_id = " << character->getDatabaseID();
            const dal::RecordSet &quests = mDb->execSql(s.str());
            const unsigned nRows = quests.rows();
            for (unsigned row = 0; row < nRows; row++)
            {
                QuestInfo quest;
                quest.id = toUint(quests(row, 0));
                quest.state = toUint(quests(row, 1));
                quest.title = quests(row, 2);
                quest.description = quests(row, 3);
                character->mQuests.push_back(quest);
            }
        }
    }
    catch (const dal::DbSqlQueryExecFailure &e)
    {
        utils::throwError("DALStorage::getCharacter #1) SQL query failure: ",
                          e);
    }

    Possessions &poss = character->getPossessions();

    try
    {
        std::ostringstream sql;
        sql << " select id, owner_id, slot, class_id, amount, equipped from "
            << INVENTORIES_TBL_NAME << " where owner_id = '"
            << character->getDatabaseID() << "' order by slot asc;";

        InventoryData inventoryData;
        EquipData equipmentData;
        const dal::RecordSet &itemInfo = mDb->execSql(sql.str());
        if (!itemInfo.isEmpty())
        {
            for (int k = 0, size = itemInfo.rows(); k < size; ++k)
            {
                InventoryItem item;
                unsigned short slot = toUint(itemInfo(k, 2));
                item.itemId   = toUint(itemInfo(k, 3));
                item.amount   = toUint(itemInfo(k, 4));
                item.equipmentSlot = toUint(itemInfo(k, 5));
                inventoryData[slot] = item;

                if (item.equipmentSlot != 0)
                    equipmentData.insert(slot);
            }
        }
        poss.setInventory(inventoryData);
        poss.setEquipment(equipmentData);
    }
    catch (const dal::DbSqlQueryExecFailure &e)
    {
        utils::throwError("DALStorage::getCharacter #3) SQL query failure: ",
                          e);
    }

    return character;
}

CharacterData *Storage::getCharacter(int id, Account *owner)
{
    std::ostringstream sql;
    sql << "SELECT * FROM " << CHARACTERS_TBL_NAME << " WHERE id = ?";
    if (mDb->prepareSql(sql.str()))
    {
        mDb->bindValue(1, id);
        return getCharacterBySQL(owner);
    }
    return 0;
}

CharacterData *Storage::getCharacter(const std::string &name)
{
    std::ostringstream sql;
    sql << "SELECT * FROM " << CHARACTERS_TBL_NAME << " WHERE name = ?";
    if (mDb->prepareSql(sql.str()))
    {
        mDb->bindValue(1, name);
        return getCharacterBySQL(0);
    }
    return 0;
}

unsigned Storage::getCharacterId(const std::string &name)
{
    std::ostringstream sql;
    sql << "SELECT id FROM " << CHARACTERS_TBL_NAME << " WHERE name = ?";
    if (!mDb->prepareSql(sql.str()))
        return 0;
    try
    {
        mDb->bindValue(1, name);
        const dal::RecordSet &charInfo = mDb->processSql();
        if (charInfo.isEmpty())
            return 0;

        string_to< unsigned > toUint;
        return toUint(charInfo(0, 0));
    }
    catch (const dal::DbSqlQueryExecFailure &e)
    {
        utils::throwError("DALStorage::getCharacterId #1) SQL query failure: ",
                          e);
    }
    return 0;
}

bool Storage::doesUserNameExist(const std::string &name)
{
    try
    {
        std::ostringstream sql;
        sql << "SELECT COUNT(username) FROM " << ACCOUNTS_TBL_NAME
            << " WHERE username = ?";

        if (mDb->prepareSql(sql.str()))
        {
            mDb->bindValue(1, name);
            const dal::RecordSet &accountInfo = mDb->processSql();

            std::istringstream ssStream(accountInfo(0, 0));
            unsigned iReturn = 1;
            ssStream >> iReturn;
            return iReturn != 0;
        }
        else
        {
            utils::throwError("(DALStorage::doesUserNameExist) "
                              "SQL query preparation failure.");
        }
    }
    catch (const std::exception &e)
    {
        utils::throwError("(DALStorage::doesUserNameExist) SQL query failure: ",
                          e);
    }

    // Should never happen
    return true;
}

bool Storage::doesEmailAddressExist(const std::string &email)
{
    try
    {
        std::ostringstream sql;
        sql << "SELECT COUNT(email) FROM " << ACCOUNTS_TBL_NAME
            << " WHERE UPPER(email) = UPPER(?)";
        if (mDb->prepareSql(sql.str()))
        {
            mDb->bindValue(1, email);
            const dal::RecordSet &accountInfo = mDb->processSql();

            std::istringstream ssStream(accountInfo(0, 0));
            unsigned iReturn = 1;
            ssStream >> iReturn;
            return iReturn != 0;
        }
        else
        {
            utils::throwError("(DALStorage::doesEmailAddressExist) "
                              "SQL query preparation failure.");
        }
    }
    catch (const std::exception &e)
    {
        utils::throwError("(DALStorage::doesEmailAddressExist) "
                          "SQL query failure: ", e);
    }

    // Should never happen
    return true;
}

bool Storage::doesCharacterNameExist(const std::string& name)
{
    try
    {
        std::ostringstream sql;
        sql << "SELECT COUNT(name) FROM " << CHARACTERS_TBL_NAME
            << " WHERE name = ?";
        if (mDb->prepareSql(sql.str()))
        {
            mDb->bindValue(1, name);

            const dal::RecordSet &accountInfo = mDb->processSql();

            std::istringstream ssStream(accountInfo(0, 0));
            int iReturn = 1;
            ssStream >> iReturn;
            return iReturn != 0;
        }
        else
        {
            utils::throwError("(DALStorage::doesCharacterNameExist) "
                              "SQL query preparation failure.");
        }
    }
    catch (const std::exception &e)
    {
        utils::throwError("(DALStorage::doesCharacterNameExist) "
                          "SQL query failure: ", e);
    }

    // Should never happen
    return true;
}

bool Storage::updateCharacter(CharacterData *character)
{
    dal::PerformTransaction transaction(mDb);

    try
    {
        // Update the database Character data (see CharacterData for details)
        std::ostringstream sqlUpdateCharacterInfo;
        sqlUpdateCharacterInfo
            << "update "        << CHARACTERS_TBL_NAME << " "
            << "set "
            << "gender = '"     << character->getGender() << "', "
            << "hair_style = '" << character->getHairStyle() << "', "
            << "hair_color = '" << character->getHairColor() << "', "
            << "char_pts = '"   << character->getAttributePoints() << "', "
            << "correct_pts = '"<< character->getCorrectionPoints() << "', "
            << "x = '"          << character->getPosition().x << "', "
            << "y = '"          << character->getPosition().y << "', "
            << "map_id = '"     << character->getMapId() << "', "
            << "slot = '"     << character->getCharacterSlot() << "' "
            << "where id = '"   << character->getDatabaseID() << "';";

        mDb->execSql(sqlUpdateCharacterInfo.str());
    }
    catch (const dal::DbSqlQueryExecFailure& e)
    {
        utils::throwError("(DALStorage::updateCharacter #1) "
                          "SQL query failure: ", e);
    }

    // Character attributes.
    try
    {
        for (AttributeMap::const_iterator it = character->mAttributes.begin(),
             it_end = character->mAttributes.end(); it != it_end; ++it)
            updateAttribute(character->getDatabaseID(), it->first,
                            it->second.base, it->second.modified);
    }
    catch (const dal::DbSqlQueryExecFailure &e)
    {
        utils::throwError("(DALStorage::updateCharacter #2) "
                          "SQL query failure: ", e);
    }

    // Character's kill count
    try
    {
        std::map<int, int>::const_iterator kill_it;
        for (kill_it = character->getKillCountBegin();
             kill_it != character->getKillCountEnd(); ++kill_it)
        {
            updateKillCount(character->getDatabaseID(),
                            kill_it->first, kill_it->second);
        }
    }
    catch (const dal::DbSqlQueryExecFailure& e)
    {
        utils::throwError("(DALStorage::updateCharacter #4) "
                          "SQL query failure: ", e);
    }

    //  Character's abillities
    try
    {
        // Out with the old
        std::ostringstream deleteSql("");
        std::ostringstream insertSql;
        deleteSql   << "DELETE FROM " << CHAR_ABILITIES_TBL_NAME
                    << " WHERE char_id='"
                    << character->getDatabaseID() << "';";
        mDb->execSql(deleteSql.str());
        // In with the new
        for (int abilityId : character->getAbilities())
        {
            insertSql.str("");
            insertSql   << "INSERT INTO " << CHAR_ABILITIES_TBL_NAME
                        << " (char_id, ability_id)"
                        << " VALUES ("
                        << " '" << character->getDatabaseID() << "',"
                        << " '" << abilityId
                        << "');";
            mDb->execSql(insertSql.str());
        }
    }
    catch (const dal::DbSqlQueryExecFailure& e)
    {
        utils::throwError("(DALStorage::updateCharacter #5) "
                          "SQL query failure: ", e);;
    }

    //  Character's questlog
    try
    {
        // Out with the old
        std::ostringstream deleteSql("");
        std::ostringstream insertSql;
        deleteSql   << "DELETE FROM " << QUESTLOG_TBL_NAME
                    << " WHERE char_id='"
                    << character->getDatabaseID() << "';";
        mDb->execSql(deleteSql.str());
        // In with the new
        for (QuestInfo &quest : character->mQuests)
        {
            insertSql.str("");
            insertSql   << "INSERT INTO " << QUESTLOG_TBL_NAME
                        << " (char_id, quest_id, quest_state, "
                        << "quest_title, quest_description)"
                        << " VALUES ("
                        << character->getDatabaseID() << ","
                        << " " << quest.id << ","
                        << " " << quest.state << ","
                        << " ?,"
                        << " ?"
                        << ")";
            if (mDb->prepareSql(insertSql.str()))
            {
                mDb->bindValue(1, quest.title);
                mDb->bindValue(2, quest.description);

                mDb->processSql();
            }
        }
    }
    catch (const dal::DbSqlQueryExecFailure& e)
    {
        utils::throwError("(DALStorage::updateCharacter #5) "
                          "SQL query failure: ", e);;
    }

    // Character's inventory
    // Delete the old inventory and equipment table first
    try
    {
        std::ostringstream sqlDeleteCharacterInventory;
        sqlDeleteCharacterInventory
            << "delete from " << INVENTORIES_TBL_NAME
            << " where owner_id = '" << character->getDatabaseID() << "';";
        mDb->execSql(sqlDeleteCharacterInventory.str());
    }
    catch (const dal::DbSqlQueryExecFailure& e)
    {
        utils::throwError("(DALStorage::updateCharacter #6) "
                          "SQL query failure: ", e);
    }

    // Insert the new inventory data
    try
    {
        std::ostringstream sql;

        sql << "insert into " << INVENTORIES_TBL_NAME
            << " (owner_id, slot, class_id, amount, equipped) values ("
            << character->getDatabaseID() << ", ";
        std::string base = sql.str();

        const Possessions &poss = character->getPossessions();
        const InventoryData &inventoryData = poss.getInventory();
        for (InventoryData::const_iterator itemIt = inventoryData.begin(),
             j_end = inventoryData.end(); itemIt != j_end; ++itemIt)
        {
            sql.str("");
            unsigned short slot = itemIt->first;
            unsigned itemId = itemIt->second.itemId;
            unsigned amount = itemIt->second.amount;
            assert(itemId);
            sql << base << slot << ", " << itemId << ", " << amount << ", "
                << itemIt->second.equipmentSlot << ");";
            mDb->execSql(sql.str());
        }

    }
    catch (const dal::DbSqlQueryExecFailure& e)
    {
        utils::throwError("(DALStorage::updateCharacter #7) "
                          "SQL query failure: ", e);
    }


    // Update char status effects
    try
    {
        // Delete the old status effects first
        std::ostringstream sql;

        sql << "delete from " << CHAR_STATUS_EFFECTS_TBL_NAME
            << " where char_id = '" << character->getDatabaseID() << "';";

         mDb->execSql(sql.str());
    }
    catch (const dal::DbSqlQueryExecFailure& e)
    {
        utils::throwError("(DALStorage::updateCharacter #8) "
                          "SQL query failure: ", e);
    }
    try
    {
        std::map<int, Status>::const_iterator status_it;
        for (status_it = character->getStatusEffectBegin();
             status_it != character->getStatusEffectEnd(); ++status_it)
        {
            insertStatusEffect(character->getDatabaseID(),
                               status_it->first, status_it->second.time);
        }
    }
    catch (const dal::DbSqlQueryExecFailure& e)
    {
        utils::throwError("(DALStorage::updateCharacter #9) "
                          "SQL query failure: ", e);
    }

    transaction.commit();
    return true;
}

void Storage::addAccount(Account *account)
{
    assert(account->getCharacters().size() == 0);

    using namespace dal;

    try
    {
        // Insert the account
        std::ostringstream sql;
        sql << "insert into " << ACCOUNTS_TBL_NAME
             << " (username, password, email, level, "
             << "banned, registration, lastlogin)"
             << " VALUES (?, ?, ?, "
             << account->getLevel() << ", 0, "
             << account->getRegistrationDate() << ", "
             << account->getLastLogin() << ");";

        if (mDb->prepareSql(sql.str()))
        {
            mDb->bindValue(1, account->getName());
            mDb->bindValue(2, account->getPassword());
            mDb->bindValue(3, account->getEmail());

            mDb->processSql();
            account->setID(mDb->getLastId());
        }
        else
        {
            utils::throwError("(DALStorage::addAccount) "
                              "SQL preparation query failure.");
        }
    }
    catch (const dal::DbSqlQueryExecFailure &e)
    {
        utils::throwError("(DALStorage::addAccount) SQL query failure: ", e);
    }
}

void Storage::flush(Account *account)
{
    assert(account->getID() >= 0);

    using namespace dal;

    try
    {
        PerformTransaction transaction(mDb);

        // Update the account
        std::ostringstream sqlUpdateAccountTable;
        sqlUpdateAccountTable
             << "update " << ACCOUNTS_TBL_NAME
             << " set username = ?, password = ?, email = ?, "
             << "level = ?, lastlogin = ? where id = ?;";

        if (mDb->prepareSql(sqlUpdateAccountTable.str()))
        {
            mDb->bindValue(1, account->getName());
            mDb->bindValue(2, account->getPassword());
            mDb->bindValue(3, account->getEmail());
            mDb->bindValue(4, account->getLevel());
            mDb->bindValue(5, account->getLastLogin());
            mDb->bindValue(6, account->getID());

            mDb->processSql();
        }
        else
        {
            utils::throwError("(DALStorage::flush) "
                              "SQL preparation query failure.");
        }

        // Get the list of characters that belong to this account.
        Characters &characters = account->getCharacters();

        // Insert or update the characters.
        for (Characters::const_iterator it = characters.begin(),
             it_end = characters.end(); it != it_end; ++it)
        {
            CharacterData *character = (*it).second;
            if (character->getDatabaseID() >= 0)
            {
                updateCharacter(character);
            }
            else
            {
                std::ostringstream sqlInsertCharactersTable;
                // Insert the character
                // This assumes that the characters name has been checked for
                // uniqueness
                sqlInsertCharactersTable
                     << "insert into " << CHARACTERS_TBL_NAME
                     << " (user_id, name, gender, hair_style, hair_color,"
                     << " char_pts, correct_pts,"
                     << " x, y, map_id, slot) values ("
                     << account->getID() << ", ?, "
                     << character->getGender() << ", "
                     << (int)character->getHairStyle() << ", "
                     << (int)character->getHairColor() << ", "
                     << (int)character->getAttributePoints() << ", "
                     << (int)character->getCorrectionPoints() << ", "
                     << character->getPosition().x << ", "
                     << character->getPosition().y << ", "
                     << character->getMapId() << ", "
                     << character->getCharacterSlot()
                     << ");";

                mDb->prepareSql(sqlInsertCharactersTable.str());
                mDb->bindValue(1, character->getName());
                mDb->processSql();

                // Update the character ID.
                character->setDatabaseID(mDb->getLastId());

                // Update all attributes.
                AttributeMap::const_iterator attr_it, attr_end;
                for (attr_it =  character->mAttributes.begin(),
                     attr_end = character->mAttributes.end();
                     attr_it != attr_end; ++attr_it)
                {
                    updateAttribute(character->getDatabaseID(), attr_it->first,
                                    attr_it->second.base,
                                    attr_it->second.modified);
                }
            }
        }

        // Existing characters in memory have been inserted
        // or updated in database.
        // Now, let's remove those who are no more in memory from database.

        string_to<unsigned short> toUint;

        std::ostringstream sqlSelectNameIdCharactersTable;
        sqlSelectNameIdCharactersTable
            << "select name, id from " << CHARACTERS_TBL_NAME
            << " where user_id = '" << account->getID() << "';";

        const RecordSet& charInMemInfo =
            mDb->execSql(sqlSelectNameIdCharactersTable.str());

        // We compare chars from memory and those existing in db,
        // and delete those not in mem but existing in db.
        bool charFound;
        for (unsigned i = 0; i < charInMemInfo.rows(); ++i) // In database
        {
            charFound = false;
            for (Characters::const_iterator it = characters.begin(),
                 it_end = characters.end(); it != it_end; ++it) // In memory
            {
                if (charInMemInfo(i, 0) == (*it).second->getName())
                {
                    charFound = true;
                    break;
                }
            }
            if (!charFound)
            {
                // The char is in db but not in memory,
                // it will be removed from database.
                // We store the id of the char to delete,
                // because as deleted, the RecordSet is also emptied,
                // and that creates an error.
                unsigned charId = toUint(charInMemInfo(i, 1));
                delCharacter(charId);
            }
        }

        transaction.commit();
    }
    catch (const std::exception &e)
    {
        utils::throwError("(DALStorage::flush) SQL query failure: ", e);
    }
}

void Storage::delAccount(Account *account)
{
    // Sync the account info into the database.
    flush(account);

    try
    {
        // Delete the account.
        std::ostringstream sql;
        sql << "delete from " << ACCOUNTS_TBL_NAME
            << " where id = '" << account->getID() << "';";
        mDb->execSql(sql.str());

        // Remove the account's characters.
        account->setCharacters(Characters());
    }
    catch (const std::exception &e)
    {
        utils::throwError("(DALStorage::delAccount) SQL query failure: ", e);
    }
}

void Storage::updateLastLogin(const Account *account)
{
    try
    {
        std::ostringstream sql;
        sql << "UPDATE " << ACCOUNTS_TBL_NAME
            << "   SET lastlogin = '" << account->getLastLogin() << "'"
            << " WHERE id = '" << account->getID() << "';";
        mDb->execSql(sql.str());
    }
    catch (const dal::DbSqlQueryExecFailure &e)
    {
        utils::throwError("(DALStorage::updateLastLogin) SQL query failure: ",
                          e);
    }
}

void Storage::updateCharacterPoints(int charId,
                                    int charPoints, int corrPoints)
{
    try
    {
        std::ostringstream sql;
        sql << "UPDATE " << CHARACTERS_TBL_NAME
            << " SET char_pts = " << charPoints << ", "
            << " correct_pts = " << corrPoints
            << " WHERE id = " << charId;

        mDb->execSql(sql.str());
    }
    catch (dal::DbSqlQueryExecFailure &e)
    {
        utils::throwError("(DALStorage::updateCharacterPoints) "
                          "SQL query failure: ", e);
    }
}

void Storage::updateAttribute(int charId, unsigned attrId,
                              double base, double mod)
{
    try
    {
        std::ostringstream sql;
        sql << "UPDATE " << CHAR_ATTR_TBL_NAME
            << " SET attr_base = '" << base << "', "
            << "attr_mod = '" << mod << "' "
            << "WHERE char_id = '" << charId << "' "
            << "AND attr_id = '" << attrId << "';";
        mDb->execSql(sql.str());

        // If this has modified a row, we're done, it updated sucessfully.
        if (mDb->getModifiedRows() > 0)
            return;

        // If it did not change anything,
        // then the record didn't previously exist. Create it.
        sql.clear();
        sql.str("");
        sql << "INSERT INTO " << CHAR_ATTR_TBL_NAME
            << " (char_id, attr_id, attr_base, attr_mod) VALUES ( "
            << charId << ", " << attrId << ", " << base << ", "
            << mod << ")";
        mDb->execSql(sql.str());
    }
    catch (const dal::DbSqlQueryExecFailure &e)
    {
        utils::throwError("(DALStorage::updateAttribute) SQL query failure: ",
                          e);
    }
}

void Storage::updateKillCount(int charId, int monsterId, int kills)
{
    try
    {
        // Try to update the kill count
        std::ostringstream sql;
        sql << "UPDATE " << CHAR_KILL_COUNT_TBL_NAME
            << " SET kills = " << kills
            << " WHERE char_id = " << charId
            << " AND monster_id = " << monsterId;
        mDb->execSql(sql.str());

        // Check if the update has modified a row
        if (mDb->getModifiedRows() > 0)
            return;

        sql.clear();
        sql.str("");
        sql << "INSERT INTO " << CHAR_KILL_COUNT_TBL_NAME << " "
            << "(char_id, monster_id, kills) VALUES ( "
            << charId << ", "
            << monsterId << ", "
            << kills << ")";
        mDb->execSql(sql.str());
    }
    catch (const dal::DbSqlQueryExecFailure &e)
    {
        utils::throwError("(DALStorage::updateKillCount) SQL query failure: ",
                          e);
    }
}

void Storage::insertStatusEffect(int charId, int statusId, int time)
{
    try
    {
        std::ostringstream sql;

        sql << "insert into " << CHAR_STATUS_EFFECTS_TBL_NAME
            << " (char_id, status_id, status_time) VALUES ( "
            << charId << ", "
            << statusId << ", "
            << time << ")";
        mDb->execSql(sql.str());
    }
    catch (const dal::DbSqlQueryExecFailure &e)
    {
        utils::throwError("(DALStorage::insertStatusEffect) "
                          "SQL query failure: ", e);
    }
}

void Storage::addGuild(Guild *guild)
{
    try
    {
        std::ostringstream sqlQuery;
        sqlQuery << "insert into " << GUILDS_TBL_NAME
                 << " (name) VALUES (?)";
        if (mDb->prepareSql(sqlQuery.str()))
        {
            mDb->bindValue(1, guild->getName());
            mDb->processSql();
        }
        else
        {
            utils::throwError("(DALStorage::addGuild) "
                              "SQL query preparation failure #1.");
        }

        sqlQuery.clear();
        sqlQuery.str("");
        sqlQuery << "SELECT id FROM " << GUILDS_TBL_NAME
                 << " WHERE name = ?";

        if (mDb->prepareSql(sqlQuery.str()))
        {
            mDb->bindValue(1, guild->getName());
            const dal::RecordSet& guildInfo = mDb->processSql();

            string_to<unsigned> toUint;
            unsigned id = toUint(guildInfo(0, 0));
            guild->setId(id);
        }
        else
        {
            utils::throwError("(DALStorage::addGuild) "
                              "SQL query preparation failure #2.");
        }
    }
    catch (const std::exception &e)
    {
        utils::throwError("(DALStorage::addGuild) SQL query failure: ", e);
    }
}

void Storage::removeGuild(Guild *guild)
{
    try
    {
        std::ostringstream sql;
        sql << "delete from " << GUILDS_TBL_NAME
            << " where id = '"
            << guild->getId() << "';";
        mDb->execSql(sql.str());
    }
    catch (const dal::DbSqlQueryExecFailure &e)
    {
        utils::throwError("(DALStorage::removeGuild) SQL query failure: ", e);
    }
}

void Storage::addGuildMember(int guildId, int memberId)
{
    try
    {
        std::ostringstream sql;
        sql << "insert into " << GUILD_MEMBERS_TBL_NAME
        << " (guild_id, member_id, rights)"
        << " values ("
        << guildId << ", \""
        << memberId << "\", "
        << 0 << ");";
        mDb->execSql(sql.str());
    }
    catch (const dal::DbSqlQueryExecFailure& e)
    {
        utils::throwError("(DALStorage::addGuildMember) SQL query failure: ",
                          e);
    }
}

void Storage::removeGuildMember(int guildId, int memberId)
{
    try
    {
        std::ostringstream sql;
        sql << "delete from " << GUILD_MEMBERS_TBL_NAME
        << " where member_id = \""
        << memberId << "\" and guild_id = '"
        << guildId << "';";
        mDb->execSql(sql.str());
    }
    catch (const dal::DbSqlQueryExecFailure& e)
    {
        utils::throwError("(DALStorage::removeGuildMember) SQL query failure: ",
                          e);
    }
}

void Storage::addFloorItem(int mapId, int itemId, int amount,
                           int posX, int posY)
{
    try
    {
        std::ostringstream sql;
        sql << "INSERT INTO " << FLOOR_ITEMS_TBL_NAME
        << " (map_id, item_id, amount, pos_x, pos_y)"
        << " VALUES ("
        << mapId << ", "
        << itemId << ", "
        << amount << ", "
        << posX << ", "
        << posY << ");";
        mDb->execSql(sql.str());
    }
    catch (const dal::DbSqlQueryExecFailure& e)
    {
        utils::throwError("(DALStorage::addFloorItem) SQL query failure: ", e);
    }
}

void Storage::removeFloorItem(int mapId, int itemId, int amount,
                                int posX, int posY)
{
    try
    {
        std::ostringstream sql;
        sql << "DELETE FROM " << FLOOR_ITEMS_TBL_NAME
        << " WHERE map_id = "
        << mapId << " AND item_id = "
        << itemId << " AND amount = "
        << amount << " AND pos_x = "
        << posX << " AND pos_y = "
        << posY << ";";
        mDb->execSql(sql.str());
    }
    catch (const dal::DbSqlQueryExecFailure& e)
    {
        utils::throwError("(DALStorage::removeFloorItem) SQL query failure: ",
                          e);
    }
}

std::list<FloorItem> Storage::getFloorItemsFromMap(int mapId)
{
    std::list<FloorItem> floorItems;

    try
    {
        std::ostringstream sql;
        sql << "SELECT * FROM " << FLOOR_ITEMS_TBL_NAME
        << " WHERE map_id = " << mapId;

        string_to< unsigned > toUint;
        const dal::RecordSet &itemInfo = mDb->execSql(sql.str());
        if (!itemInfo.isEmpty())
        {
            for (int k = 0, size = itemInfo.rows(); k < size; ++k)
            {
                floorItems.push_back(FloorItem(toUint(itemInfo(k, 2)),
                                                toUint(itemInfo(k, 3)),
                                                toUint(itemInfo(k, 4)),
                                                toUint(itemInfo(k, 5))));
            }
        }
    }
    catch (const dal::DbSqlQueryExecFailure &e)
    {
        utils::throwError("DALStorage::getFloorItemsFromMap "
            "SQL query failure: ", e);
    }

    return floorItems;
}

void Storage::setMemberRights(int guildId, int memberId, int rights)
{
    try
    {
        std::ostringstream sql;
        sql << "UPDATE " << GUILD_MEMBERS_TBL_NAME
            << " SET rights = " << rights
            << " WHERE member_id = " << memberId
            << "  AND guild_id = " << guildId << ";";
        mDb->execSql(sql.str());
    }
    catch (const dal::DbSqlQueryExecFailure& e)
    {
        utils::throwError("(DALStorage::setMemberRights) SQL query failure: ",
                          e);
    }
}

std::map<int, Guild*> Storage::getGuildList()
{
    std::map<int, Guild*> guilds;
    std::stringstream sql;
    string_to<short> toShort;


    // Get the guilds stored in the db.
    try
    {
        sql << "select id, name from " << GUILDS_TBL_NAME << ";";
        const dal::RecordSet& guildInfo = mDb->execSql(sql.str());

        // Check that at least 1 guild was returned
        if (guildInfo.isEmpty())
            return guilds;

        // Loop through every row in the table and assign it to a guild
        for (unsigned i = 0; i < guildInfo.rows(); ++i)
        {
            Guild* guild = new Guild(guildInfo(i,1));
            guild->setId(toShort(guildInfo(i,0)));
            guilds[guild->getId()] = guild;
        }
        string_to< unsigned > toUint;

        // Add the members to the guilds.
        for (std::map<int, Guild*>::iterator it = guilds.begin();
             it != guilds.end(); ++it)
        {
            std::ostringstream memberSql;
            memberSql << "select member_id, rights from "
                      << GUILD_MEMBERS_TBL_NAME
                      << " where guild_id = '" << it->second->getId() << "';";
            const dal::RecordSet& memberInfo = mDb->execSql(memberSql.str());

            std::list<std::pair<int, int> > members;
            for (unsigned j = 0; j < memberInfo.rows(); ++j)
            {
                members.push_back(std::pair<int, int>(toUint(memberInfo(j, 0)),
                                                     toUint(memberInfo(j, 1))));
            }

            std::list<std::pair<int, int> >::const_iterator i, i_end;
            for (i = members.begin(), i_end = members.end(); i != i_end; ++i)
            {
                CharacterData *character = getCharacter((*i).first, 0);
                if (character)
                {
                    character->addGuild(it->second->getName());
                    it->second->addMember(character->getDatabaseID(), (*i).second);
                }
            }
        }
    }
    catch (const dal::DbSqlQueryExecFailure& e)
    {
        utils::throwError("(DALStorage::getGuildList) SQL query failure: ", e);
    }

    return guilds;
}

std::string Storage::getQuestVar(int id, const std::string &name)
{
    try
    {
        std::ostringstream query;
        query << "select value from " << QUESTS_TBL_NAME
                << " WHERE owner_id = ? AND name = ?";
        if (mDb->prepareSql(query.str()))
        {
            mDb->bindValue(1, id);
            mDb->bindValue(2, name);
            const dal::RecordSet &info = mDb->processSql();

            if (!info.isEmpty())
                return info(0, 0);
        }
        else
        {
            utils::throwError("(DALStorage:getQuestVar) "
                              "SQL query preparation failure.");
        }
    }
    catch (const dal::DbSqlQueryExecFailure &e)
    {
        utils::throwError("(DALStorage::getQuestVar) SQL query failure: ", e);
    }

    // Should never happen
    return std::string();
}

std::string Storage::getWorldStateVar(const std::string &name, int mapId)
{
    try
    {
        std::ostringstream query;
        query << "SELECT `value` "
              << "FROM " << WORLD_STATES_TBL_NAME
              << " WHERE `state_name` = ?";

        // Add map filter if map_id is given
        if (mapId >= 0)
            query << " AND `map_id` = ?";

        //query << ";"; <-- No ';' at the end of prepared statements.

        if (mDb->prepareSql(query.str()))
        {
            mDb->bindValue(1, name);
            if (mapId >= 0)
                mDb->bindValue(2, mapId);
            const dal::RecordSet &info = mDb->processSql();

            if (!info.isEmpty())
                return info(0, 0);
        }
        else
        {
            utils::throwError("(DALStorage:getWorldStateVar) "
                              "SQL query preparation failure.");
        }
    }
    catch (const dal::DbSqlQueryExecFailure &e)
    {
        utils::throwError("(DALStorage::getWorldStateVar) SQL query failure: ",
                          e);
    }

    return std::string();
}

std::map<std::string, std::string> Storage::getAllWorldStateVars(int mapId)
{
    std::map<std::string, std::string> variables;

    // Avoid a crash because prepared statements must have at least one binding.
    if (mapId < 0)
    {
        LOG_ERROR("getAllWorldStateVars was called with a negative map Id: "
                  << mapId);
        return variables;
    }

    try
    {
        std::ostringstream query;
        query << "SELECT `state_name`, `value` "
              << "FROM " << WORLD_STATES_TBL_NAME;

        // Add map filter if map_id is given
        if (mapId >= 0)
            query << " WHERE `map_id` = ?";

        //query << ";"; <-- No ';' at the end of prepared statements.

        if (mDb->prepareSql(query.str()))
        {
            if (mapId >= 0)
                mDb->bindValue(1, mapId);
            const dal::RecordSet &results = mDb->processSql();

            for (unsigned i = 0; i < results.rows(); ++i)
            {
                variables[results(i, 0)] = results(i, 1);
            }
        }
        else
        {
            utils::throwError("(DALStorage:getAllWorldStateVar) "
                              "SQL query preparation failure.");
        }
    }
    catch (const dal::DbSqlQueryExecFailure &e)
    {
        utils::throwError("(DALStorage::getWorldStateVar) SQL query failure: ",
                          e);
    }

    return variables;
}

void Storage::setWorldStateVar(const std::string &name,
                               const std::string &value,
                               int mapId)
{
    try
    {
        // Set the value to empty means: delete the variable
        if (value.empty())
        {
            std::ostringstream deleteStateVar;
            deleteStateVar << "DELETE FROM " << WORLD_STATES_TBL_NAME
                           << " WHERE state_name = '" << name << "'"
                           << " AND map_id = '" << mapId << "';";
            mDb->execSql(deleteStateVar.str());
            return;
        }

        // Try to update the variable in the database
        std::ostringstream updateStateVar;
        updateStateVar << "UPDATE " << WORLD_STATES_TBL_NAME
                       << "   SET value = '" << value << "', "
                       << "       moddate = '" << time(0) << "' "
                       << " WHERE state_name = '" << name << "'"
                       << " AND map_id = '" << mapId << "';";
        mDb->execSql(updateStateVar.str());

        // If we updated a row, were finished here
        if (mDb->getModifiedRows() > 0)
            return;

        // Otherwise we have to add the new variable
        std::ostringstream insertStateVar;
        insertStateVar << "INSERT INTO " << WORLD_STATES_TBL_NAME
                       << " (state_name, map_id, value , moddate) VALUES ("
                       << "'" << name << "', "
                       << "'" << mapId << "', "
                       << "'" << value << "', "
                       << "'" << time(0) << "');";
        mDb->execSql(insertStateVar.str());
    }
    catch (const dal::DbSqlQueryExecFailure &e)
    {
        utils::throwError("(DALStorage::setWorldStateVar) SQL query failure: ",
                          e);
    }
}

void Storage::setQuestVar(int id, const std::string &name,
                          const std::string &value)
{
    try
    {
        std::ostringstream query1;
        query1 << "delete from " << QUESTS_TBL_NAME
               << " where owner_id = '" << id << "' and name = '"
               << name << "';";
        mDb->execSql(query1.str());

        if (value.empty())
            return;

        std::ostringstream query2;
        query2 << "insert into " << QUESTS_TBL_NAME
               << " (owner_id, name, value) values ('"
               << id << "', '" << name << "', '" << value << "');";
        mDb->execSql(query2.str());
    }
    catch (const dal::DbSqlQueryExecFailure &e)
    {
        utils::throwError("(DALStorage::setQuestVar) SQL query failure: ", e);
    }
}

void Storage::banCharacter(int id, int duration)
{
    try
    {
        // check the account of the character
        std::ostringstream query;
        query << "select user_id from " << CHARACTERS_TBL_NAME
              << " where id = '" << id << "';";
        const dal::RecordSet &info = mDb->execSql(query.str());
        if (info.isEmpty())
        {
            LOG_ERROR("Tried to ban an unknown user.");
            return;
        }

        uint64_t bantime = (uint64_t)time(0) + (uint64_t)duration * 60u;
        // ban the character
        std::ostringstream sql;
        sql << "update " << ACCOUNTS_TBL_NAME
            << " set level = '" << AL_BANNED << "', banned = '"
            << bantime
            << "' where id = '" << info(0, 0) << "';";
        mDb->execSql(sql.str());
    }
    catch (const dal::DbSqlQueryExecFailure &e)
    {
        utils::throwError("(DALStorage::banAccount) SQL query failure: ", e);
    }
}

void Storage::delCharacter(int charId) const
{
    try
    {
        dal::PerformTransaction transaction(mDb);
        std::ostringstream sql;

        // Delete the inventory of the character
        sql << "DELETE FROM " << INVENTORIES_TBL_NAME
            << " WHERE owner_id = '" << charId << "';";
        mDb->execSql(sql.str());

        // Delete from the quests table
        sql.clear();
        sql.str("");
        sql << "DELETE FROM " << QUESTS_TBL_NAME
            << " WHERE owner_id = '" << charId << "';";
        mDb->execSql(sql.str());

        // Delete from the guilds table
        sql.clear();
        sql.str("");
        sql << "DELETE FROM " << GUILD_MEMBERS_TBL_NAME
            << " WHERE member_id = '" << charId << "';";
        mDb->execSql(sql.str());

        // Delete auctions of the character
        sql.clear();
        sql.str("");
        sql << "DELETE FROM " << AUCTION_TBL_NAME
            << " WHERE char_id = '" << charId << "';";
        mDb->execSql(sql.str());

        // Delete bids made on auctions made by the character
        sql.clear();
        sql.str("");
        sql << "DELETE FROM " << AUCTION_BIDS_TBL_NAME
            << " WHERE char_id = '" << charId << "';";
        mDb->execSql(sql.str());

        // Now delete the character itself.
        sql.clear();
        sql.str("");
        sql << "DELETE FROM " << CHARACTERS_TBL_NAME
            << " WHERE id = '" << charId << "';";
        mDb->execSql(sql.str());

        transaction.commit();
    }
    catch (const dal::DbSqlQueryExecFailure &e)
    {
        utils::throwError("(DALStorage::delCharacter) SQL query failure: ", e);
    }
}

void Storage::delCharacter(CharacterData *character) const
{
    delCharacter(character->getDatabaseID());
}

void Storage::checkBannedAccounts()
{
    try
    {
        // Update expired bans
        std::ostringstream sql;
        sql << "update " << ACCOUNTS_TBL_NAME
        << " set level = " << AL_PLAYER << ", banned = 0"
        << " where level = " << AL_BANNED
        << " AND banned <= " << time(0) << ";";
        mDb->execSql(sql.str());
    }
    catch (const dal::DbSqlQueryExecFailure &e)
    {
        utils::throwError("(DALStorage::checkBannedAccounts) "
                          "SQL query failure: ", e);
    }
}

void Storage::setAccountLevel(int id, int level)
{
    try
    {
        std::ostringstream sql;
        sql << "update " << ACCOUNTS_TBL_NAME
        << " set level = " << level
        << " where id = " << id << ";";
        mDb->execSql(sql.str());
    }
    catch (const dal::DbSqlQueryExecFailure &e)
    {
        utils::throwError("(DALStorage::setAccountLevel) SQL query failure: ",
                          e);
    }
}

void Storage::storeLetter(Letter *letter)
{
    try
    {
        std::ostringstream sql;
        if (letter->getId() == 0)
        {
            // The letter was never saved before
            sql << "INSERT INTO " << POST_TBL_NAME << " VALUES ( "
                << "NULL, "
                << letter->getSender()->getDatabaseID() << ", "
                << letter->getReceiver()->getDatabaseID() << ", "
                << letter->getExpiry() << ", "
                << time(0) << ", "
                << "?)";
            if (mDb->prepareSql(sql.str()))
            {
                mDb->bindValue(1, letter->getContents());
                mDb->processSql();

                letter->setId(mDb->getLastId());

                // TODO: Store attachments in the database

                return;
            }
            else
            {
                utils::throwError("(DALStorage::storeLetter) "
                                  "SQL query preparation failure #1.");
            }
        }
        else
        {
            // The letter has a unique id, update the record in the db
            sql << "UPDATE " << POST_TBL_NAME
                << "   SET sender_id       = '"
                << letter->getSender()->getDatabaseID() << "', "
                << "       receiver_id     = '"
                << letter->getReceiver()->getDatabaseID() << "', "
                << "       letter_type     = '" << letter->getType() << "', "
                << "       expiration_date = '" << letter->getExpiry() << "', "
                << "       sending_date    = '" << time(0) << "', "
                << "       letter_text = ? "
                << " WHERE letter_id       = '" << letter->getId() << "'";

            if (mDb->prepareSql(sql.str()))
            {
                mDb->bindValue(1, letter->getContents());

                mDb->processSql();

                if (mDb->getModifiedRows() == 0)
                {
                    // This should never happen...
                    utils::throwError("(DALStorage::storePost) "
                                      "trying to update nonexistant letter.");
                }

                // TODO: Update attachments in the database
            }
            else
            {
                utils::throwError("(DALStorage::storeLetter) "
                                  "SQL query preparation failure #2.");
            }
        }
    }
    catch (const std::exception &e)
    {
        utils::throwError("(DALStorage::storeLetter) Exception failure: ", e);
    }
}

Post *Storage::getStoredPost(int playerId)
{
    Post *p = new Post();

    string_to< unsigned > toUint;

    try
    {
        std::ostringstream sql;
        sql << "SELECT * FROM " << POST_TBL_NAME
            << " WHERE receiver_id = " << playerId;

        const dal::RecordSet &post = mDb->execSql(sql.str());

        if (post.isEmpty())
        {
            // There is no post waiting for the character
            return p;
        }

        for (unsigned i = 0; i < post.rows(); i++ )
        {
            // Load sender and receiver
            CharacterData *sender = getCharacter(toUint(post(i, 1)), 0);
            CharacterData *receiver = getCharacter(toUint(post(i, 2)), 0);

            Letter *letter = new Letter(toUint( post(0,3) ), sender, receiver);

            letter->setId( toUint(post(0, 0)) );
            letter->setExpiry( toUint(post(0, 4)) );
            letter->addText( post(0, 6) );

            // TODO: Load attachments per letter from POST_ATTACHMENTS_TBL_NAME
            // needs redesign of struct ItemInventroy

            p->addLetter(letter);
        }
    }
    catch (const std::exception &e)
    {
        utils::throwError("(DALStorage::getStoredPost) Exception failure: ", e);
    }

    return p;
}

void Storage::deletePost(Letter *letter)
{
    try
    {
        dal::PerformTransaction transaction(mDb);
        std::ostringstream sql;

        // First delete all attachments of the letter
        // This could leave "dead" items in the item_instances table
        sql << "DELETE FROM " << POST_ATTACHMENTS_TBL_NAME
            << " WHERE letter_id = " << letter->getId();
        mDb->execSql(sql.str());

        // Delete the letter itself
        sql.clear();
        sql.str("");
        sql << "DELETE FROM " << POST_TBL_NAME
            << " WHERE letter_id = " << letter->getId();
        mDb->execSql(sql.str());

        transaction.commit();
        letter->setId(0);
    }
    catch (const dal::DbSqlQueryExecFailure &e)
    {
        utils::throwError("(DALStorage::deletePost) SQL query failure: ", e);
    }
}

void Storage::syncDatabase()
{
    XML::Document doc(DEFAULT_ITEM_FILE);
    xmlNodePtr rootNode = doc.rootNode();

    if (!rootNode || !xmlStrEqual(rootNode->name, BAD_CAST "items"))
    {
        std::ostringstream errMsg;
        errMsg << "Item Manager: Error while loading item database"
               << "(" << DEFAULT_ITEM_FILE << ")!";
       // LOG_ERROR(errMsg);
        return;
    }

    dal::PerformTransaction transaction(mDb);
    int itemCount = 0;
    for_each_xml_child_node(node, rootNode)
    {
        // Try to load the version of the item database.
        if (xmlStrEqual(node->name, BAD_CAST "version"))
        {
            std::string revision = XML::getProperty(node, "revision",
                                                    std::string());
            mItemDbVersion = atoi(revision.c_str());
            LOG_INFO("Loading item database version " << mItemDbVersion);
        }

        if (!xmlStrEqual(node->name, BAD_CAST "item"))
            continue;

        int id = XML::getProperty(node, "id", 0);

        if (id < 1)
            continue;

        int weight = XML::getProperty(node, "weight", 0);
        std::string type = XML::getProperty(node, "type", std::string());
        std::string name = XML::getProperty(node, "name", std::string());
        std::string desc = XML::getProperty(node, "description",
                                            std::string());
        std::string eff  = XML::getProperty(node, "effect", std::string());
        std::string image = XML::getProperty(node, "image", std::string());
        std::string dye;

        // Split image name and dye string
        size_t pipe = image.find("|");
        if (pipe != std::string::npos)
        {
            dye = image.substr(pipe + 1);
            image = image.substr(0, pipe);
        }

        try
        {
            std::ostringstream sql;
            sql << "UPDATE " << ITEMS_TBL_NAME
                << " SET name = ?, "
                << "     description = ?, "
                << "     image = '" << image << "', "
                << "     weight = " << weight << ", "
                << "     itemtype = '" << type << "', "
                << "     effect = ?, "
                << "     dyestring = '" << dye << "' "
                << " WHERE id = " << id;

            if (mDb->prepareSql(sql.str()))
            {
                mDb->bindValue(1, name);
                mDb->bindValue(2, desc);
                mDb->bindValue(3, eff);

                mDb->processSql();
                if (mDb->getModifiedRows() == 0)
                {
                    sql.clear();
                    sql.str("");
                    sql << "INSERT INTO " << ITEMS_TBL_NAME
                        << "  VALUES ( " << id << ", ?, ?, '"
                        << image << "', " << weight << ", '"
                        << type << "', ?, '" << dye << "' )";
                    if (mDb->prepareSql(sql.str()))
                    {
                        mDb->bindValue(1, name);
                        mDb->bindValue(2, desc);
                        mDb->bindValue(3, eff);
                        mDb->processSql();
                    }
                    else
                    {
                        utils::throwError("(DALStorage::SyncDatabase) "
                                          "SQL query preparation failure #2.");
                    }
                }
            }
            else
            {
                utils::throwError("(DALStorage::SyncDatabase) "
                                  "SQL query preparation failure #1.");
            }
            itemCount++;
        }
        catch (const dal::DbSqlQueryExecFailure &e)
        {
            utils::throwError("(DALStorage::SyncDatabase) "
                              "SQL query failure: ", e);
        }
    }

    transaction.commit();
}

void Storage::setOnlineStatus(int charId, bool online)
{
    try
    {
        std::ostringstream sql;
        if (online)
        {
            // First we try to update the online status. this prevents errors
            // in case we get the online status twice
            sql << "SELECT COUNT(*) FROM " << ONLINE_USERS_TBL_NAME
                << " WHERE char_id = " << charId;
            const std::string res = mDb->execSql(sql.str())(0, 0);

            if (res != "0")
                return;

            sql.clear();
            sql.str("");
            sql << "INSERT INTO " << ONLINE_USERS_TBL_NAME
                << " VALUES (" << charId << ", " << time(0) << ")";
            mDb->execSql(sql.str());
        }
        else
        {
            sql << "DELETE FROM " << ONLINE_USERS_TBL_NAME
                << " WHERE char_id = " << charId;
            mDb->execSql(sql.str());
        }


    }
    catch (const dal::DbSqlQueryExecFailure &e)
    {
        utils::throwError("(DALStorage::setOnlineStatus) SQL query failure: ",
                          e);
    }
}

void Storage::addTransaction(const Transaction &trans)
{
    try
    {
        std::stringstream sql;
        sql << "INSERT INTO " << TRANSACTION_TBL_NAME
            << " VALUES (NULL, " << trans.mCharacterId << ", "
            << trans.mAction << ", "
            << "?, "
            << time(0) << ")";
        if (mDb->prepareSql(sql.str()))
        {
            mDb->bindValue(1, trans.mMessage);
            mDb->processSql();
        }
        else
        {
            utils::throwError("(DALStorage::SyncDatabase) "
                              "SQL query preparation failure.");
        }
    }
    catch (const dal::DbSqlQueryExecFailure &e)
    {
        utils::throwError("(DALStorage::addTransaction) SQL query failure: ",
                          e);
    }
}

std::vector<Transaction> Storage::getTransactions(unsigned num)
{
    std::vector<Transaction> transactions;
    string_to<unsigned> toUint;

    try
    {
        std::stringstream sql;
        sql << "SELECT * FROM " << TRANSACTION_TBL_NAME;
        const dal::RecordSet &rec = mDb->execSql(sql.str());

        int size = rec.rows();
        int start = size - num;
        // Get the last <num> records and store them in transactions
        for (int i = start; i < size; ++i)
        {
            Transaction trans;
            trans.mCharacterId = toUint(rec(i, 1));
            trans.mAction = toUint(rec(i, 2));
            trans.mMessage = rec(i, 3);
            transactions.push_back(trans);
        }
    }
    catch (const dal::DbSqlQueryExecFailure &e)
    {
        utils::throwError("(DALStorage::getTransactions) SQL query failure: ",
                          e);
    }

    return transactions;
}

std::vector<Transaction> Storage::getTransactions(time_t date)
{
    std::vector<Transaction> transactions;
    string_to<unsigned> toUint;

    try
    {
        std::stringstream sql;
        sql << "SELECT * FROM " << TRANSACTION_TBL_NAME << " WHERE time > "
            << date;
        const dal::RecordSet &rec = mDb->execSql(sql.str());

        for (unsigned i = 0; i < rec.rows(); ++i)
        {
            Transaction trans;
            trans.mCharacterId = toUint(rec(i, 1));
            trans.mAction = toUint(rec(i, 2));
            trans.mMessage = rec(i, 3);
            transactions.push_back(trans);
        }
    }
    catch (const dal::DbSqlQueryExecFailure &e)
    {
        utils::throwError("(DALStorage::getTransactions) SQL query failure: ",
                          e);
    }

    return transactions;
}
