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
#include <sstream>
#include <list>

#include "account-server/serverhandler.h"

#include "account-server/accountclient.h"
#include "account-server/accounthandler.h"
#include "account-server/character.h"
#include "account-server/flooritem.h"
#include "account-server/mapmanager.h"
#include "account-server/storage.h"
#include "chat-server/chathandler.h"
#include "chat-server/post.h"
#include "common/configuration.h"
#include "common/defines.h"
#include "common/manaserv_protocol.h"
#include "common/transaction.h"
#include "net/connectionhandler.h"
#include "net/messageout.h"
#include "net/netcomputer.h"
#include "utils/logger.h"
#include "utils/tokendispenser.h"

using namespace ManaServ;

struct MapStatistics
{
  std::vector<int> players;
  unsigned short nbEntities;
  unsigned short nbMonsters;
};

typedef std::map<unsigned short, MapStatistics> ServerStatistics;

/**
 * Stores address, maps, and statistics, of a connected game server.
 */
struct GameServer: NetComputer
{
    GameServer(ENetPeer *peer): NetComputer(peer), server(0), port(0) {}

    std::string name;
    std::string address;
    NetComputer *server;
    ServerStatistics maps;
    short port;
};

static GameServer *getGameServerFromMap(int);

/**
 * Manages communications with all the game servers.
 */
class ServerHandler: public ConnectionHandler
{
    friend GameServer *getGameServerFromMap(int);
    friend void GameServerHandler::dumpStatistics(std::ostream &);

    protected:
        /**
         * Processes server messages.
         */
        void processMessage(NetComputer *computer, MessageIn &message);

        /**
         * Called when a game server connects. Initializes a simple NetComputer
         * as these connections are stateless.
         */
        NetComputer *computerConnected(ENetPeer *peer);

        /**
         * Called when a game server disconnects.
         */
        void computerDisconnected(NetComputer *comp);
};

static ServerHandler *serverHandler;

bool GameServerHandler::initialize(int port, const std::string &host)
{
    MapManager::initialize(DEFAULT_MAPSDB_FILE);
    serverHandler = new ServerHandler;
    LOG_INFO("Game server handler started:");
    return serverHandler->startListen(port, host);
}

void GameServerHandler::deinitialize()
{
    serverHandler->stopListen();
    delete serverHandler;
}

void GameServerHandler::process()
{
    serverHandler->process(50);
}

NetComputer *ServerHandler::computerConnected(ENetPeer *peer)
{
    return new GameServer(peer);
}

void ServerHandler::computerDisconnected(NetComputer *comp)
{
    LOG_INFO("Game-server disconnected.");
    delete comp;
}

static GameServer *getGameServerFromMap(int mapId)
{
    for (ServerHandler::NetComputers::const_iterator
         i = serverHandler->clients.begin(),
         i_end = serverHandler->clients.end(); i != i_end; ++i)
    {
        GameServer *server = static_cast< GameServer * >(*i);
        ServerStatistics::const_iterator it = server->maps.find(mapId);
        if (it == server->maps.end()) continue;
        return server;
    }
    return nullptr;
}

bool GameServerHandler::getGameServerFromMap(int mapId,
                                             std::string &address,
                                             int &port)
{
    if (GameServer *s = ::getGameServerFromMap(mapId))
    {
        address = s->address;
        port = s->port;
        return true;
    }
    return false;
}

static void registerGameClient(GameServer *s, const std::string &token,
                               CharacterData *ptr)
{
    MessageOut msg(AGMSG_PLAYER_ENTER);
    msg.writeString(token, MAGIC_TOKEN_LENGTH);
    msg.writeInt32(ptr->getDatabaseID());
    msg.writeString(ptr->getName());
    ptr->serialize(msg);
    s->send(msg);
}

void GameServerHandler::registerClient(const std::string &token,
                                       CharacterData *ptr)
{
    GameServer *s = ::getGameServerFromMap(ptr->getMapId());
    assert(s);
    registerGameClient(s, token, ptr);
}

void ServerHandler::processMessage(NetComputer *comp, MessageIn &msg)
{
    GameServer *server = static_cast<GameServer *>(comp);

    switch (msg.getId())
    {
        case GAMSG_REGISTER:
        {
            LOG_DEBUG("GAMSG_REGISTER");
            // TODO: check the credentials of the game server
            server->name = msg.readString();
            server->address = msg.readString();
            server->port = msg.readInt16();
            const std::string password = msg.readString();

            // checks the version of the remote item database with our local copy
            unsigned dbversion = msg.readInt32();
            LOG_INFO("Game server uses itemsdatabase with version " << dbversion);

            LOG_DEBUG("AGMSG_REGISTER_RESPONSE");
            MessageOut outMsg(AGMSG_REGISTER_RESPONSE);
            if (dbversion == storage->getItemDatabaseVersion())
            {
                LOG_DEBUG("Item databases between account server and "
                    "gameserver are in sync");
                outMsg.writeInt16(DATA_VERSION_OK);
            }
            else
            {
                LOG_DEBUG("Item database of game server has a wrong version");
                outMsg.writeInt16(DATA_VERSION_OUTDATED);
            }
            if (password == Configuration::getValue("net_password", "changeMe"))
            {
                outMsg.writeInt16(PASSWORD_OK);

                // transmit global world state variables
                std::map<std::string, std::string> variables;
                variables = storage->getAllWorldStateVars(Storage::WorldMap);

                for (auto &variableIt : variables)
                {
                    outMsg.writeString(variableIt.first);
                    outMsg.writeString(variableIt.second);
                }

                comp->send(outMsg);
            }
            else
            {
                LOG_INFO("The password given by " << server->address << ':'
                         << server->port << " was bad.");
                outMsg.writeInt16(PASSWORD_BAD);
                comp->disconnect(outMsg);
                break;
            }

            LOG_INFO("Game server " << server->address << ':' << server->port
                     << " asks for maps to activate.");

            const std::map<int, std::string> &maps = MapManager::getMaps();
            for (std::map<int, std::string>::const_iterator it = maps.begin(),
                 it_end = maps.end(); it != it_end; ++it)
            {
                int id = it->first;
                const std::string &reservedServer = it->second;
                if (reservedServer == server->name)
                {
                    MessageOut outMsg(AGMSG_ACTIVE_MAP);

                    // Map variables
                    outMsg.writeInt16(id);
                    LOG_DEBUG("Issued server " << server->name << "("
                              << server->address << ":" << server->port << ") "
                              << "to enable map " << id);
                    std::map<std::string, std::string> variables;
                    variables = storage->getAllWorldStateVars(id);

                     // Map vars number
                    outMsg.writeInt16(variables.size());

                    for (auto &variableIt : variables)
                    {
                        outMsg.writeString(variableIt.first);
                        outMsg.writeString(variableIt.second);
                    }

                    // Persistent Floor Items
                    std::list<FloorItem> items;
                    items = storage->getFloorItemsFromMap(id);

                    outMsg.writeInt16(items.size()); //number of floor items

                    // Send each map item: item_id, amount, pos_x, pos_y
                    for (std::list<FloorItem>::iterator i = items.begin();
                         i != items.end(); ++i)
                    {
                        outMsg.writeInt32(i->getItemId());
                        outMsg.writeInt16(i->getItemAmount());
                        outMsg.writeInt16(i->getPosX());
                        outMsg.writeInt16(i->getPosY());
                    }

                    comp->send(outMsg);
                    MapStatistics &m = server->maps[id];
                    m.nbEntities = 0;
                    m.nbMonsters = 0;
                }
            }
        } break;

        case GAMSG_PLAYER_DATA:
        {
            LOG_DEBUG("GAMSG_PLAYER_DATA");
            int id = msg.readInt32();
            if (CharacterData *ptr = storage->getCharacter(id, nullptr))
            {
                ptr->deserialize(msg);
                if (!storage->updateCharacter(ptr))
                {
                    LOG_ERROR("Failed to update character "
                              << id << '.');
                }
                delete ptr;
            }
            else
            {
                LOG_ERROR("Received data for non-existing character "
                          << id << '.');
            }
        } break;

        case GAMSG_PLAYER_SYNC:
        {
            LOG_DEBUG("GAMSG_PLAYER_SYNC");
            GameServerHandler::syncDatabase(msg);
        } break;

        case GAMSG_REDIRECT:
        {
            LOG_DEBUG("GAMSG_REDIRECT");
            int id = msg.readInt32();
            std::string magic_token(utils::getMagicToken());
            if (CharacterData *ptr = storage->getCharacter(id, nullptr))
            {
                int mapId = ptr->getMapId();
                if (GameServer *s = getGameServerFromMap(mapId))
                {
                    registerGameClient(s, magic_token, ptr);
                    MessageOut result(AGMSG_REDIRECT_RESPONSE);
                    result.writeInt32(id);
                    result.writeString(magic_token, MAGIC_TOKEN_LENGTH);
                    result.writeString(s->address);
                    result.writeInt16(s->port);
                    comp->send(result);
                }
                else
                {
                    LOG_ERROR("Server Change: No game server for map " <<
                              mapId << '.');
                }
                delete ptr;
            }
            else
            {
                LOG_ERROR("Received data for non-existing character "
                          << id << '.');
            }
        } break;

        case GAMSG_PLAYER_RECONNECT:
        {
            LOG_DEBUG("GAMSG_PLAYER_RECONNECT");
            int id = msg.readInt32();
            std::string magic_token = msg.readString(MAGIC_TOKEN_LENGTH);

            if (CharacterData *ptr = storage->getCharacter(id, nullptr))
            {
                int accountID = ptr->getAccountID();
                AccountClientHandler::prepareReconnect(magic_token, accountID);
                delete ptr;
            }
            else
            {
                LOG_ERROR("Received data for non-existing character "
                          << id << '.');
            }
        } break;

        case GAMSG_GET_VAR_CHR:
        {
            int id = msg.readInt32();
            std::string name = msg.readString();
            std::string value = storage->getQuestVar(id, name);
            MessageOut result(AGMSG_GET_VAR_CHR_RESPONSE);
            result.writeInt32(id);
            result.writeString(name);
            result.writeString(value);
            comp->send(result);
        } break;

        case GAMSG_SET_VAR_CHR:
        {
            int id = msg.readInt32();
            std::string name = msg.readString();
            std::string value = msg.readString();
            storage->setQuestVar(id, name, value);
        } break;

        case GAMSG_SET_VAR_WORLD:
        {
            std::string name = msg.readString();
            std::string value = msg.readString();
            // save the new value to the database
            storage->setWorldStateVar(name, value, Storage::WorldMap);
            // relay the new value to all gameservers
            for (NetComputer *netComputer : clients)
            {
                MessageOut varUpdateMessage(AGMSG_SET_VAR_WORLD);
                varUpdateMessage.writeString(name);
                varUpdateMessage.writeString(value);
                netComputer->send(varUpdateMessage);
            }
        } break;

        case GAMSG_SET_VAR_MAP:
        {
            int mapid = msg.readInt32();
            std::string name = msg.readString();
            std::string value = msg.readString();
            storage->setWorldStateVar(name, value, mapid);
        } break;

        case GAMSG_BAN_PLAYER:
        {
            int id = msg.readInt32();
            int duration = msg.readInt32();
            storage->banCharacter(id, duration);
        } break;

        case GAMSG_CHANGE_ACCOUNT_LEVEL:
        {
            int id = msg.readInt32();
            int level = msg.readInt16();

            // get the character so we can get the account id
            CharacterData *c = storage->getCharacter(id, nullptr);
            if (c)
            {
                storage->setAccountLevel(c->getAccountID(), level);
            }
        } break;

        case GAMSG_STATISTICS:
        {
            while (msg.getUnreadLength())
            {
                int mapId = msg.readInt16();
                ServerStatistics::iterator i = server->maps.find(mapId);
                if (i == server->maps.end())
                {
                    LOG_ERROR("Server " << server->address << ':'
                              << server->port << " should not be sending stati"
                              "stics for map " << mapId << '.');
                    // Skip remaining data.
                    break;
                }
                MapStatistics &m = i->second;
                m.nbEntities = msg.readInt16();
                m.nbMonsters = msg.readInt16();
                int nb = msg.readInt16();
                m.players.resize(nb);
                for (int j = 0; j < nb; ++j)
                {
                    m.players[j] = msg.readInt32();
                }
            }
        } break;

        case GCMSG_REQUEST_POST:
        {
            // Retrieve the post for user
            LOG_DEBUG("GCMSG_REQUEST_POST");
            MessageOut result(CGMSG_POST_RESPONSE);

            // get the character id
            int characterId = msg.readInt32();

            // send the character id of sender
            result.writeInt32(characterId);

            // get the character based on the id
            CharacterData *ptr = storage->getCharacter(characterId, nullptr);
            if (!ptr)
            {
                // Invalid character
                LOG_ERROR("Error finding character id for post");
                break;
            }

            // get the post for that character
            Post *post = postalManager->getPost(ptr);

            // send the post if valid
            if (post)
            {
                for (unsigned i = 0; i < post->getNumberOfLetters(); ++i)
                {
                    // get each letter, send the sender's name,
                    // the contents and any attachments
                    Letter *letter = post->getLetter(i);
                    result.writeString(letter->getSender()->getName());
                    result.writeString(letter->getContents());
                    std::vector<InventoryItem> items = letter->getAttachments();
                    for (unsigned j = 0; j < items.size(); ++j)
                    {
                        result.writeInt16(items[j].itemId);
                        result.writeInt16(items[j].amount);
                    }
                }

                // clean up
                postalManager->clearPost(ptr);
            }

            comp->send(result);
        } break;

        case GCMSG_STORE_POST:
        {
            // Store the letter for the user
            LOG_DEBUG("GCMSG_STORE_POST");
            MessageOut result(CGMSG_STORE_POST_RESPONSE);

            // get the sender and receiver
            int senderId = msg.readInt32();
            std::string receiverName = msg.readString();

            // for sending it back
            result.writeInt32(senderId);

            // get their characters
            CharacterData *sender = storage->getCharacter(senderId, nullptr);
            CharacterData *receiver = storage->getCharacter(receiverName);
            if (!sender || !receiver)
            {
                // Invalid character
                LOG_ERROR("Error finding character id for post");
                result.writeInt8(ERRMSG_INVALID_ARGUMENT);
                break;
            }

            // get the letter contents
            std::string contents = msg.readString();

            std::vector< std::pair<int, int> > items;
            while (msg.getUnreadLength())
            {
                items.push_back(std::pair<int, int>(msg.readInt16(), msg.readInt16()));
            }

            // save the letter
            LOG_DEBUG("Creating letter");
            Letter *letter = new Letter(0, sender, receiver);
            letter->addText(contents);
            for (unsigned i = 0; i < items.size(); ++i)
            {
                InventoryItem item;
                item.itemId = items[i].first;
                item.amount = items[i].second;
                letter->addAttachment(item);
            }
            postalManager->addLetter(letter);

            result.writeInt8(ERRMSG_OK);
            comp->send(result);
        } break;

        case GAMSG_TRANSACTION:
        {
            LOG_DEBUG("TRANSACTION");
            int id = msg.readInt32();
            int action = msg.readInt32();
            std::string message = msg.readString();

            Transaction trans;
            trans.mCharacterId = id;
            trans.mAction = action;
            trans.mMessage = message;
            storage->addTransaction(trans);
        } break;

        case GCMSG_PARTY_INVITE:
            chatHandler->handlePartyInvite(msg);
            break;

        case GAMSG_CREATE_ITEM_ON_MAP:
        {
            int mapId = msg.readInt32();
            int itemId = msg.readInt32();
            int amount = msg.readInt16();
            int posX = msg.readInt16();
            int posY = msg.readInt16();

            LOG_DEBUG("Gameserver create item " << itemId
                << " on map " << mapId);

            storage->addFloorItem(mapId, itemId, amount, posX, posY);
        } break;

        case GAMSG_REMOVE_ITEM_ON_MAP:
        {
            int mapId = msg.readInt32();
            int itemId = msg.readInt32();
            int amount = msg.readInt16();
            int posX = msg.readInt16();
            int posY = msg.readInt16();

            LOG_DEBUG("Gameserver removed item " << itemId
                << " from map " << mapId);

            storage->removeFloorItem(mapId, itemId, amount, posX, posY);
        } break;

        case GAMSG_ANNOUNCE:
        {
            const std::string message = msg.readString();
            const int senderId = msg.readInt16();
            const std::string senderName = msg.readString();
            chatHandler->handleAnnounce(message, senderId, senderName);
        } break;

        default:
            LOG_WARN("ServerHandler::processMessage, Invalid message type: "
                     << msg.getId());
            MessageOut result(XXMSG_INVALID);
            comp->send(result);
            break;
    }
}

void GameServerHandler::dumpStatistics(std::ostream &os)
{
    for (ServerHandler::NetComputers::const_iterator
         i = serverHandler->clients.begin(),
         i_end = serverHandler->clients.end(); i != i_end; ++i)
    {
        GameServer *server = static_cast< GameServer * >(*i);
        if (!server->port)
            continue;

        os << "<gameserver address=\"" << server->address << "\" port=\""
           << server->port << "\">\n";

        for (ServerStatistics::const_iterator j = server->maps.begin(),
             j_end = server->maps.end(); j != j_end; ++j)
        {
            const MapStatistics &m = j->second;
            os << "<map id=\"" << j->first << "\" nb_entities=\"" << m.nbEntities
               << "\" nb_monsters=\"" << m.nbMonsters << "\">\n";
            for (std::vector< int >::const_iterator k = m.players.begin(),
                 k_end = m.players.end(); k != k_end; ++k)
            {
                os << "<character id=\"" << *k << "\"/>\n";
            }
            os << "</map>\n";
        }
        os << "</gameserver>\n";
    }
}

void GameServerHandler::sendPartyChange(CharacterData *ptr, int partyId)
{
    GameServer *s = ::getGameServerFromMap(ptr->getMapId());
    if (s)
    {
        MessageOut msg(CGMSG_CHANGED_PARTY);
        msg.writeInt32(ptr->getDatabaseID());
        msg.writeInt32(partyId);
        s->send(msg);
    }
}

void GameServerHandler::syncDatabase(MessageIn &msg)
{
    // It is safe to perform the following updates in a transaction
    dal::PerformTransaction transaction(storage->database());

    while (msg.getUnreadLength() > 0)
    {
        int msgType = msg.readInt8();
        switch (msgType)
        {
            case SYNC_CHARACTER_POINTS:
            {
                LOG_DEBUG("received SYNC_CHARACTER_POINTS");
                int charId = msg.readInt32();
                int charPoints = msg.readInt32();
                int corrPoints = msg.readInt32();
                storage->updateCharacterPoints(charId, charPoints, corrPoints);
            } break;

            case SYNC_CHARACTER_ATTRIBUTE:
            {
                LOG_DEBUG("received SYNC_CHARACTER_ATTRIBUTE");
                int    charId = msg.readInt32();
                int    attrId = msg.readInt32();
                double base   = msg.readDouble();
                double mod    = msg.readDouble();
                storage->updateAttribute(charId, attrId, base, mod);
            } break;

            case SYNC_ONLINE_STATUS:
            {
                LOG_DEBUG("received SYNC_ONLINE_STATUS");
                int charId = msg.readInt32();
                bool online = (msg.readInt8() == 1);
                storage->setOnlineStatus(charId, online);
            } break;
        }
    }

    transaction.commit();
}
