/*
 *  The Mana Server
 *  Copyright (C) 2008-2010  The Mana World Development Team
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

#include <sstream>

#include "commandhandler.h"
#include "accountconnection.h"
#include "charactercomponent.h"
#include "effect.h"
#include "gamehandler.h"
#include "inventory.h"
#include "item.h"
#include "itemmanager.h"
#include "mapcomposite.h"
#include "mapmanager.h"
#include "monster.h"
#include "monstermanager.h"
#include "abilitymanager.h"
#include "state.h"

#include "scripting/scriptmanager.h"

#include "common/configuration.h"
#include "common/permissionmanager.h"
#include "common/transaction.h"

#include "utils/string.h"

struct CmdRef
{
    const char *cmd;
    const char *usage;
    const char *help;
    void (*func)(Entity*, std::string&) ;
};

static void handleHelp(Entity*, std::string&);
static void handleReport(Entity*, std::string&);
static void handleWhere(Entity*, std::string&);
static void handleRights(Entity*, std::string&);
static void handleWarp(Entity*, std::string&);
static void handleCharWarp(Entity*, std::string&);
static void handleGoto(Entity*, std::string&);
static void handleRecall(Entity*, std::string&);
static void handleBan(Entity*, std::string&);
static void handleItem(Entity*, std::string&);
static void handleDrop(Entity*, std::string&);
static void handleMoney(Entity*, std::string&);
static void handleSpawn(Entity*, std::string&);
static void handleAttribute(Entity*, std::string&);
static void handleReload(Entity*, std::string&);
static void handlePermissions(Entity*, std::string&);
static void handleGivePermission(Entity*, std::string&);
static void handleTakePermission(Entity*, std::string&);
static void handleAnnounce(Entity*, std::string&);
static void handleHistory(Entity*, std::string&);
static void handleMute(Entity*, std::string&);
static void handleDie(Entity*, std::string&);
static void handleKill(Entity*, std::string&);
static void handleKick(Entity*, std::string&);
static void handleLog(Entity*, std::string&);
static void handleLogsay(Entity*, std::string&);
static void handleKillMonsters(Entity*, std::string&);
static void handleCraft(Entity*, std::string&);
static void handleGetPos(Entity*, std::string&);
static void handleEffect(Entity*, std::string&);
static void handleGiveAbility(Entity*, std::string&);
static void handleTakeAbility(Entity*, std::string&);
static void handleRechargeAbility(Entity*, std::string&);
static void handleListAbility(Entity*, std::string&);
static void handleSetAttributePoints(Entity*, std::string&);
static void handleSetCorrectionPoints(Entity*, std::string&);

static CmdRef const cmdRef[] =
{
    {"help",   "[command]" ,
        "Lists all available commands or a detailed help for a command", &handleHelp },
    {"report", "<bug>"     ,
        "Sends a bug or abuse reports a bug to the server administration", &handleReport},
    {"where",  ""          ,
        "Tells you your location in the game world", &handleWhere},
    {"rights", ""          ,
        "Tells you your current permissions", &handleRights},
    {"warp",   "<map> <x> <y>",
        "Teleports your character to a different location in the game world", &handleWarp},
    {"charwarp",   "<character> <map> <x> <y>",
        "Teleports the given character to a different location in the game world", &handleCharWarp},
    {"goto", "<character>",
        "Teleports you to the location of another character", &handleGoto},
    {"recall", "<character>",
        "Teleports another character to your location", &handleRecall},
    {"ban", "<character> <length of time>(m|h|d|w|y)",
        "Bans the character and all characters on the same account from the game", &handleBan},
    {"item", "<character> <item id> <amount>",
        "Creates a number of items in the inventory of a character", &handleItem},
    {"drop", "<item id> <amount>",
        "Drops a stack of items on the ground at your current location", &handleDrop},
    {"money", "<character> <amount>",
        "Changes the money a character possesses", &handleMoney},
    {"spawn", "<monster id> <number>",
        "Creates a number of monsters near your location", &handleSpawn},
    {"attribute", "<character> <attribute> <value>",
        "Changes the character attributes of a character", &handleAttribute},
    {"reload", "",
        "Makes the server reload all configuration files", &handleReload},
    {"permissions", "",
        "Tells you the permissions of another player", &handlePermissions},
    {"givepermission", "<character> <permission class>",
        "Gives a permission class to the account a character belongs to", &handleGivePermission},
    {"takepermission", "<character> <permission class>",
        "Takes a permission class from the account a character belongs to", &handleTakePermission},
    {"announce", "<message>",
        "Sends a chat message to all characters in the game", &handleAnnounce},
    {"history", "<number of transactions>",
        "Shows the last transactions", &handleHistory},
    {"mute","<character> <length in seconds>",
        "Prevents the character from talking for the specified number of seconds. Use 0 seconds to unmute.", &handleMute},
    {"die", "",
        "Kills you.", &handleDie},
    {"kill", "<character>",
        "Kills the character.", &handleKill},
    {"kick", "<character>",
        "Disconnects the client of character.", &handleKick},
    {"log", "<message>",
        "Logs a message to the transaction log.", &handleLog},
    {"logsay", "<message>",
        "Says something in public chat while logging it to the transaction log.", &handleLogsay},
    {"killmonsters", "",
        "Kills all monsters on the map.", &handleKillMonsters},
    {"craft", "{ <item> <amount> }",
        "Crafts something.", &handleCraft},
    {"getpos", "<character>",
        "Gets the position of a character.", &handleGetPos},
    {"effect", "<effectid> <x> <y> / <effectid> <being> / <effectid>",
        "Shows an effect at the given position or on the given being. "
        "The player's character is targeted if neither of them is provided.",
        &handleEffect},
    {"giveability", "<character> <ability>",
        "Gives the character the ability. "
        "The ability can get passed as abilityid or in the format "
        "<setname>_<abilityname>", &handleGiveAbility},
    {"takeability", "<character> <ability>",
        "Takes the ability aways from the character. "
        "The ability can get passed as abilityid or in the format "
        "<setname>_<abilityname>", &handleTakeAbility},
    {"rechargeability", "<character> <ability>",
        "Recharges the ability of the character. "
        "The ability can get passed as abilityid or in the format "
        "<setname>_<abilityname>", &handleRechargeAbility},
    {"listabilities", "<character>",
        "Lists the abilitys of the character.", &handleListAbility},
    {"setattributepoints", "<character> <amount>",
        "Sets the attribute points of a character.", &handleSetAttributePoints},
    {"setcorrectionpoints", "<character> <amount>",
        "Sets the correction points of a character.", &handleSetCorrectionPoints},
    {nullptr, nullptr, nullptr, nullptr}

};

static void say(const std::string &message, Entity *player)
{
    GameState::sayTo(player, nullptr, message);
}

/*
static bool checkPermission(Character *player, unsigned permissions)
{
    if (player->getAccountLevel() & permissions)
    {
        return true;
    }

    say("Invalid permissions", player);

    return false;
}*/

/**
 * Returns the next argument, and remove it from the given string.
 */

static std::string playerRights(Entity *ch)
{
    std::stringstream str;
    str << (unsigned)ch->getComponent<CharacterComponent>()->getAccountLevel();
    str << " ( ";
    std::list<std::string> classes = PermissionManager::getClassList(ch);

    for (std::string &permission : classes)
    {
        str << permission << " ";
    }
    str << ")";
    return str.str();
}

static std::string getArgument(std::string &args)
{
    std::string argument;
    std::string::size_type pos = std::string::npos;
    bool doubleQuotes = false;

    // Finds out if the next argument is between double-quotes
    if (args.empty() || args.at(0) != '"')
    {
        // No double-quotes, we then search an ending space.
        pos = args.find(' ');
        doubleQuotes = false;
    }
    else
    {
        // Exclude the first double-quote from search.
        pos = args.find('"', 1);
        doubleQuotes = true;
    }

    if (pos != std::string::npos)
    {
        argument = args.substr(0, pos);
        if (doubleQuotes)
        {
            // Jumps to the next parameter,
            // after the ending double-quote and space,
            // and remove the two double-quotes before returning.
            if (pos + 2 < args.size())
            {
                args = args.substr(pos + 2);
            }
            else
            {
                // This was the last argument
                args.clear();
            }
            argument = argument.substr(1, pos - 1);
        }
        else
        {
            // Jumps to the next parameter, after the ending space.
            args = args.substr(pos + 1);
        }
    }
    else
    {
        argument = args;
        args = std::string();
    }
    return argument;
}

static void handleHelp(Entity *player, std::string &args)
{
    if (args.empty())
    {
        // short list of all commands
        say("=Available Commands=", player);
        for (std::string &command :
                PermissionManager::getPermissionList(player))
        {
            say(command, player);
        }
    } else {
        // don't show help for commands the player may not use
        if (PermissionManager::checkPermission(player, "@"+args) == PermissionManager::PMR_DENIED)
        {
            say("Why do you want to know? You can't use it anyway!", player);
            return;
        }
        // detailed description of single command
        for (size_t j = 0; cmdRef[j].cmd != nullptr; j++)
        {
            if (cmdRef[j].cmd == args)
            {
                std::string msg;
                msg.append("@");
                msg.append(cmdRef[j].cmd);
                msg.append(" ");
                msg.append(cmdRef[j].usage);
                say(msg, player);
                say(cmdRef[j].help, player);
                return;
            }
        }
        say("There is no command @"+args, player);
    }
}

static void handleWarp(Entity *player, std::string &args)
{
    int x, y;
    MapComposite *map;

    // get the arguments
    std::string mapstr = getArgument(args);
    std::string xstr = getArgument(args);
    std::string ystr = getArgument(args);

    // if any of them are empty strings, no argument was given
    if (mapstr.empty() || xstr.empty() || ystr.empty())
    {
        say("Invalid number of arguments given.", player);
        say("Usage: @warp <map> <x> <y>", player);
        return;
    }

    // if it contains # then it means the player's map
    if (mapstr == "#")
    {
        map = player->getMap();
    }
    else
    {
        if (mapstr[0] == '#')
        {
            mapstr = mapstr.substr(1);
            // check for valid map id
            int id;
            if (!utils::isNumeric(mapstr))
            {
                say("Invalid map", player);
                return;
            }

            id = utils::stringToInt(mapstr);

            // get the map
            map = MapManager::getMap(id);
            if (!map)
            {
                say("Invalid map", player);
                return;
            }
        }
        else
        {
            map = MapManager::getMap(mapstr);

            if (!map)
            {
                say("Invalid map", player);
                return;
            }
        }
    }

    if (!utils::isNumeric(xstr))
    {
        say("Invalid x", player);
        return;
    }

    if (!utils::isNumeric(ystr))
    {
        say("Invalid y", player);
        return;
    }

    // change the x and y to integers
    x = utils::stringToInt(xstr);
    y = utils::stringToInt(ystr);

    // now warp the player
    GameState::warp(player, map, Point(x, y));

    // log transaction
    std::stringstream ss;
    ss << "User warped to " << map->getName() << " (" << x << ", " << y << ")";
    int databaseId =
            player->getComponent<CharacterComponent>()->getDatabaseID();
    accountHandler->sendTransaction(databaseId, TRANS_CMD_WARP,
                                    ss.str());
}

static void handleCharWarp(Entity *player, std::string &args)
{
    int x, y;
    MapComposite *map;
    Entity *other;

    // get the arguments
    std::string character = getArgument(args);
    std::string mapstr = getArgument(args);
    std::string xstr = getArgument(args);
    std::string ystr = getArgument(args);

    // if any of them are empty strings, no argument was given
    if (character.empty() || mapstr.empty() || xstr.empty() || ystr.empty())
    {
        say("Invalid number of arguments given.", player);
        say("Usage: @warp <character> <map> <x> <y>", player);
        return;
    }

    // if it contains # then it means the player
    if (character == "#")
    {
        other = player;
    }
    else
    {
        // check for valid player
        other = gameHandler->getCharacterByNameSlow(character);
        if (!other)
        {
            say("Invalid or offline character <" + character + ">.", player);
            return;
        }
    }

    // if it contains # then it means the player's map
    if (mapstr == "#")
    {
        map = player->getMap();
    }
    else
    {
        if (mapstr[0] == '#')
        {
            mapstr = mapstr.substr(1);
            // check for valid map id
            int id;
            if (!utils::isNumeric(mapstr))
            {
                say("Invalid map", player);
                return;
            }

            id = utils::stringToInt(mapstr);

            // get the map
            map = MapManager::getMap(id);
            if (!map)
            {
                say("Invalid map", player);
                return;
            }
        }
        else
        {
            map = MapManager::getMap(mapstr);

            if (!map)
            {
                say("Invalid map", player);
                return;
            }
        }
    }

    if (!utils::isNumeric(xstr))
    {
        say("Invalid x", player);
        return;
    }

    if (!utils::isNumeric(ystr))
    {
        say("Invalid y", player);
        return;
    }

    // change the x and y to integers
    x = utils::stringToInt(xstr);
    y = utils::stringToInt(ystr);

    // now warp the player
    GameState::warp(other, map, Point(x, y));

    // log transaction
    std::stringstream ss;
    ss << "User warped " << other->getComponent<BeingComponent>()->getName()
            << " to " << map->getName() << " (" << x << ", " << y << ")";
    int databaseId =
            player->getComponent<CharacterComponent>()->getDatabaseID();
    accountHandler->sendTransaction(databaseId, TRANS_CMD_WARP, ss.str());
}

static void handleItem(Entity *player, std::string &args)
{
    Entity *other;
    ItemClass *ic;
    int value = 0;

    // get arguments
    std::string character = getArgument(args);
    std::string itemclass = getArgument(args);
    std::string valuestr = getArgument(args);

    // check all arguments are there
    if (character.empty() || itemclass.empty())
    {
        say("Invalid number of arguments given.", player);
        say("Usage: @item <character> <item> [amount]", player);
        return;
    }

    // if it contains # that means the player
    if (character == "#")
    {
        other = player;
    }
    else
    {
        // check for valid player
        other = gameHandler->getCharacterByNameSlow(character);
        if (!other)
        {
            say("Invalid character or they are offline", player);
            return;
        }
    }

    // identify the item type
    if (utils::isNumeric(itemclass))
    {
        int id = utils::stringToInt(itemclass);
        ic = itemManager->getItem(id);
    }
    else
    {
        ic = itemManager->getItemByName(itemclass);
    }
    if (!ic)
    {
        say("Invalid item", player);
        return;
    }

    //identify the amount
    if  (valuestr.empty())
    {
        value = 1;
    }
    else if (utils::isNumeric(valuestr))
    {
        value = utils::stringToInt(valuestr);
    }
    // check for valid amount
    if (value <= 0)
    {
        say("Invalid number of items", player);
        return;
    }

    // insert the item into the inventory
    Inventory(other).insert(ic->getDatabaseID(), value);

    // log transaction
    std::stringstream str;
    str << "User created item " << ic->getDatabaseID();
    int databaseId =
            player->getComponent<CharacterComponent>()->getDatabaseID();
    accountHandler->sendTransaction(databaseId, TRANS_CMD_ITEM, str.str());
}

static void handleDrop(Entity *player, std::string &args)
{
    ItemClass *ic;
    int amount = 0;

    // get arguments
    std::string itemclass = getArgument(args);
    std::string amountstr = getArgument(args);

    // check all arguments are there
    if (itemclass.empty())
    {
        say("Invalid number of arguments given.", player);
        say("Usage: @drop <item> [amount]", player);
        return;
    }

    // identify the item type
    if (utils::isNumeric(itemclass))
    {
        int id = utils::stringToInt(itemclass);
        ic = itemManager->getItem(id);
    }
    else
    {
        ic = itemManager->getItemByName(itemclass);
    }
    if (!ic)
    {
        say("Invalid item", player);
        return;
    }

    // identify the amount
    if (amountstr.empty())
    {
        amount = 1;
    }
    else if (utils::isNumeric(amountstr))
    {
        amount = utils::stringToInt(amountstr);
    }
    // check for valid amount
    if (amount <= 0)
    {
        say("Invalid number of items", player);
        return;
    }

    const Point &position =
            player->getComponent<ActorComponent>()->getPosition();
    Entity *item = Item::create(player->getMap(), position, ic, amount);

    GameState::insertOrDelete(item);

    // log transaction
    std::stringstream str;
    str << "User created item " << ic->getDatabaseID();
    int databaseId =
            player->getComponent<CharacterComponent>()->getDatabaseID();
    accountHandler->sendTransaction(databaseId, TRANS_CMD_DROP, str.str());
}

static void handleMoney(Entity *player, std::string &args)
{
    Entity *other;
    int value;

    // get arguments
    std::string character = getArgument(args);
    std::string valuestr = getArgument(args);

    // check all arguments are there
    if (character.empty() || valuestr.empty())
    {
        say("Invalid number of arguments given", player);
        say("Usage: @money <character> <amount>", player);
        return;
    }

    // check if its the player itself
    if (character == "#")
    {
        other = player;
    }
    else
    {
        // check for valid player
        other = gameHandler->getCharacterByNameSlow(character);
        if (!other)
        {
            say("Invalid character or they are offline", player);
            return;
        }
    }

    // check value is an integer
    if (!utils::isNumeric(valuestr))
    {
        say("Invalid argument", player);
        return;
    }

    // change value into an integer
    value = utils::stringToInt(valuestr);

    auto *beingComponent = other->getComponent<BeingComponent>();

    auto *moneyAttribute = attributeManager->getAttributeInfo(ATTR_GP);

    // change how much money the player has
    const double previousMoney = beingComponent->getAttributeBase(moneyAttribute);
    beingComponent->setAttribute(*player, moneyAttribute , previousMoney + value);

    // log transaction
    std::string msg = "User created " + valuestr + " money";
    int databaseId =
            player->getComponent<CharacterComponent>()->getDatabaseID();
    accountHandler->sendTransaction(databaseId, TRANS_CMD_MONEY, msg);
}

static void handleSpawn(Entity *player, std::string &args)
{
    MonsterClass *mc;
    MapComposite *map = player->getMap();
    const Point &pos = player->getComponent<ActorComponent>()->getPosition();
    int value = 0;

    // get the arguments
    std::string monsterclass = getArgument(args);
    std::string valuestr = getArgument(args);

    // check all arguments are there
    if (monsterclass.empty())
    {
        say("Invalid amount of arguments given.", player);
        say("Usage: @spawn <monster> [number]", player);
        return;
    }

    // identify the monster type
    if (utils::isNumeric(monsterclass))
    {
        int id = utils::stringToInt(monsterclass);
        mc = monsterManager->getMonster(id);
    }
    else
    {
        mc = monsterManager->getMonsterByName(monsterclass);
    }
    // check for valid monster
    if (!mc)
    {
        say("Invalid monster", player);
        return;
    }

    //identify the amount
    if  (valuestr.empty())
    {
        value = 1;
    }
    else if (utils::isNumeric(valuestr))
    {
        value = utils::stringToInt(valuestr);
    }
    // check for valid amount
    if (value <= 0)
    {
        say("Invalid number of monsters", player);
        return;
    }

    // create the monsters and put them on the map
    for (int i = 0; i < value; ++i)
    {
        Entity *monster = new Entity(OBJECT_MONSTER);
        auto *actorComponent = new ActorComponent(*monster);
        monster->addComponent(actorComponent);
        actorComponent->setPosition(*monster, pos);
        monster->addComponent(new BeingComponent(*monster));
        monster->addComponent(new MonsterComponent(*monster, mc));
        monster->setMap(map);
        if (!GameState::insertOrDelete(monster))
        {
            // The map is full. Break out.
            break;
        }

        // log transaction
        std::string msg = "User created monster " +
                monster->getComponent<BeingComponent>()->getName();
        int databaseId =
                player->getComponent<CharacterComponent>()->getDatabaseID();
        accountHandler->sendTransaction(databaseId, TRANS_CMD_SPAWN, msg);
    }
}

static void handleGoto(Entity *player, std::string &args)
{
    Entity *other;

    // get the arguments
    std::string character = getArgument(args);

    // check all arguments are there
    if (character.empty())
    {
        say("Invalid amount of arguments given.", player);
        say("Usage: @goto <character>", player);
        return;
    }

    // check for valid player
    other = gameHandler->getCharacterByNameSlow(character);
    if (!other)
    {
        say("Invalid character, or player is offline.", player);
        return;
    }

    // move the player to where the other player is
    MapComposite *map = other->getMap();
    const Point &pos = other->getComponent<ActorComponent>()->getPosition();
    GameState::warp(player, map, pos);

    // log transaction
    std::stringstream msg;
    msg << "User warped own character to "
            << other->getComponent<BeingComponent>()->getName();
    int databaseId =
            player->getComponent<CharacterComponent>()->getDatabaseID();
    accountHandler->sendTransaction(databaseId, TRANS_CMD_GOTO, msg.str());
}

static void handleRecall(Entity *player, std::string &args)
{
    Entity *other;

    // get the arguments
    std::string character = getArgument(args);

    // check all arguments are there
    if (character.empty())
    {
        say("Invalid amount of arguments given.", player);
        say("Usage: @recall <character>", player);
        return;
    }

    // check for valid player
    other = gameHandler->getCharacterByNameSlow(character);
    if (!other)
    {
        say("Invalid character, or player is offline.", player);
        return;
    }

    // move the other player to where the player is
    MapComposite *map = player->getMap();
    const Point &pos = player->getComponent<ActorComponent>()->getPosition();
    GameState::warp(other, map, pos);
}

static void handleReload(Entity *, std::string &)
{
    // reload the items and monsters
    itemManager->reload();
    monsterManager->reload();
}

static void handleBan(Entity *player, std::string &args)
{
    Entity *other;
    int length;
    int lengthMutiplier = 0;

    // get arguments
    std::string character = getArgument(args);
    std::string valuestr = getArgument(args);

    // check all arguments are there
    if (character.empty() || valuestr.empty())
    {
        say("Invalid number of arguments given.", player);
        say("Usage: @ban <character> <duration>", player);
        return;
    }

    // check for valid player
    other = gameHandler->getCharacterByNameSlow(character);
    if (!other)
    {
        say("Invalid character", player);
        return;
    }

    // get the unit
    char unit = valuestr.at(valuestr.length()-1);
    switch (unit)
    {
        case 'm':
            lengthMutiplier = 1;
            break;
        case 'h':
            lengthMutiplier = 60;
            break;
        case 'd':
            lengthMutiplier = 60 * 24;
            break;
        case 'w':
            lengthMutiplier = 60 * 24 * 7;
            break;
        case 'y':
            lengthMutiplier = 60 * 24 * 365;
            break;
    }
    length = utils::stringToInt(valuestr.substr(0, valuestr.length()-1));
    length = length * lengthMutiplier;
    if (length <= 0)
    {
        std::string errmsg;
        errmsg += "Invalid length. Please enter a positive number ";
        errmsg += "followed by the letter m, h, d, w or y for minutes ";
        errmsg += ", hours, days, weeks or years.";
        say(errmsg , player);
        return;
    }

    auto *characterComponent = player->getComponent<CharacterComponent>();

    // ban the player
    accountHandler->banCharacter(other, length);
    // disconnect the player
    MessageOut kickmsg(GPMSG_CONNECT_RESPONSE);
    kickmsg.writeInt8(ERRMSG_ADMINISTRATIVE_LOGOFF);
    characterComponent->getClient()->disconnect(kickmsg);

    // feedback for command user
    std::string otherName = other->getComponent<BeingComponent>()->getName();
    std::string msg = "You've banned " + otherName + " for " + utils::toString(length) + " minutes";
    say(msg, player);
    // log transaction
    msg = "User banned " + otherName + " for " + utils::toString(length) + " minutes";
    accountHandler->sendTransaction(characterComponent->getDatabaseID(),
                                    TRANS_CMD_BAN, msg);
}

static void handlePermissions(Entity *player, std::string &args)
{
    std::string character = getArgument(args);
    if (character.empty())
    {
        say("Invaild number of arguments given.", player);
        say("Usage: @permissions <character>", player);
        return;
    }

    Entity *other = gameHandler->getCharacterByNameSlow(character);
    if (!other)
    {
        say("Invalid character", player);
        return;
    }

    say(other->getComponent<BeingComponent>()->getName() + " has the permissions: " +
        playerRights(other), player);
}

static void handleGivePermission(Entity *player, std::string &args)
{
    Entity *other;

    // get the arguments
    std::string character = getArgument(args);
    std::string strPermission = getArgument(args);

    // check all arguments are there
    if (character.empty() || strPermission.empty())
    {
        say("Invalid number of arguments given.", player);
        say("Usage: @givepermission <character> <permission class>", player);
        return;
    }

    // check if its to effect the player
    if (character == "#")
    {
        other = player;
    }
    else
    {
        // check for valid player
        other = gameHandler->getCharacterByNameSlow(character);
        if (!other)
        {
            say("Invalid character", player);
            return;
        }
    }

    unsigned char permission = PermissionManager::getMaskFromAlias(strPermission);

    if (permission == 0x00)
    {
        say ("Unknown permission class: "+strPermission, player);
        return;
    }

    auto *characterComponent =
            player->getComponent<CharacterComponent>();

    if (permission & characterComponent->getAccountLevel())
    {
        say(player->getComponent<BeingComponent>()->getName()
            +" already has the permission "+strPermission, player);
    }
    else
    {
        permission += characterComponent->getAccountLevel();
        // change the player's account level
        characterComponent->setAccountLevel(permission);
        accountHandler->changeAccountLevel(other, permission);

        // log transaction
        std::string msg = "User gave right " + strPermission + " to " +
                other->getComponent<BeingComponent>()->getName();
        accountHandler->sendTransaction(characterComponent->getDatabaseID(),
                                        TRANS_CMD_SETGROUP, msg);
        say("You gave " + other->getComponent<BeingComponent>()->getName() +
            " the rights of a " + strPermission, player);
        say("Congratulations, " +
            player->getComponent<BeingComponent>()->getName() +
            " gave you the rights of a " + strPermission, other);
    }
}

static void handleTakePermission(Entity *player, std::string &args)
{
    Entity *other;

    // get the arguments
    std::string character = getArgument(args);
    std::string strPermission = getArgument(args);

    // check all arguments are there
    if (character.empty() || strPermission.empty())
    {
        say("Invalid number of arguments given.", player);
        say("Usage: @takepermission <character> <permission class>", player);
        return;
    }

    // check if its to effect the player
    if (character == "#")
    {
        other = player;
    }
    else
    {
        // check for valid player
        other = gameHandler->getCharacterByNameSlow(character);
        if (!other)
        {
            say("Invalid character", player);
            return;
        }
    }

    unsigned char permission = PermissionManager::getMaskFromAlias(strPermission);

    if (permission == 0x00)
    {
        say("Unknown permission class: "+strPermission, player);
        return;
    }

    auto *characterComponent =
            player->getComponent<CharacterComponent>();

    if (!(permission & characterComponent->getAccountLevel()))
    {
        say(player->getComponent<BeingComponent>()->getName()
            +" hasn't got the permission "+strPermission, player);
    } else {
        permission = characterComponent->getAccountLevel() - permission;
        // change the player's account level
        characterComponent->setAccountLevel(permission);
        accountHandler->changeAccountLevel(other, permission);

        // log transaction
        std::string msg = "User took right " + strPermission + " from "
                + other->getComponent<BeingComponent>()->getName();
        accountHandler->sendTransaction(characterComponent->getDatabaseID(),
                                        TRANS_CMD_SETGROUP, msg);
        say("Sorry, "+player->getComponent<BeingComponent>()->getName()
            +" revoked your rights of a "+strPermission, other);
    }
}


static void handleAttribute(Entity *player, std::string &args)
{
    Entity *other;

    // get arguments
    std::string character = getArgument(args);
    std::string attribute = getArgument(args);
    std::string valuestr = getArgument(args);

    // check all arguments are there
    if (character.empty() || valuestr.empty() || attribute.empty())
    {
        say("Invalid number of arguments given.", player);
        say("Usage: @attribute <character> <attribute> <value>", player);
        return;
    }

    // check if its the player or another player
    if (character == "#")
    {
        other = player;
    }
    else
    {
        // check for valid player
        other = gameHandler->getCharacterByNameSlow(character);
        if (!other)
        {
            say("Invalid character", player);
            return;
        }
    }

    // check they are really integers
    if (!utils::isNumeric(valuestr))
    {
        say("Invalid argument", player);
        return;
    }

    AttributeInfo *attributeInfo;
    if (utils::isNumeric(attribute))
    {
        const int id = utils::stringToInt(attribute);
        attributeInfo = attributeManager->getAttributeInfo(id);
    }
    else
    {
        attributeInfo = attributeManager->getAttributeInfo(attribute);
    }

    if (!attributeInfo)
    {
        say("Invalid Attribute", player);
        return;
    }

    // put the value into an integer
    int value = utils::stringToInt(valuestr);

    if (value < 0)
    {
        say("Invalid amount", player);
        return;
    }

    auto *beingComponent = other->getComponent<BeingComponent>();

    if (!attributeInfo)
    {
        say("Invalid attribute", player);
        return;
    }

    // change the player's attribute
    beingComponent->setAttribute(*other, attributeInfo, value);

    // log transaction
    std::stringstream msg;
    msg << "User changed attribute " << attributeInfo->id << " of player "
        << beingComponent->getName()
        << " to " << value;
    int databaseId =
            player->getComponent<CharacterComponent>()->getDatabaseID();
    accountHandler->sendTransaction(databaseId, TRANS_CMD_ATTRIBUTE, msg.str());
}

static void handleReport(Entity *player, std::string &args)
{
    std::string bugReport = getArgument(args);

    if (bugReport.empty())
    {
        say("Invalid number of arguments given.", player);
        say("Usage: @report <message>", player);
        return;
    }

    // TODO: Send the report to a developer or something
}

static void handleAnnounce(Entity *player, std::string &args)
{
    if (args.empty())
    {
        say("Invalid number of arguments given.", player);
        say("Usage: @announce <message>", player);
        return;
    }

    MessageOut msg(GAMSG_ANNOUNCE);
    msg.writeString(args);
    msg.writeInt16(player->getComponent<CharacterComponent>()
                   ->getDatabaseID());
    msg.writeString(player->getComponent<BeingComponent>()->getName());
    accountHandler->send(msg);
}

static void handleWhere(Entity *player, std::string &)
{
    const Point &position =
            player->getComponent<ActorComponent>()->getPosition();
    std::stringstream str;
    str << "Your current location is map "
        << player->getMap()->getID()
        << " ["
        << position.x
        << ":"
        << position.y
        << "]";
    say (str.str(), player);
}

static void handleRights(Entity *player, std::string &)
{
    say("Your rights level is: " + playerRights(player), player);
}

static void handleHistory(Entity *, std::string &)
{
    // TODO: Get args number of transactions and show them to the player
}

static void handleMute(Entity *player, std::string &args)
{
    Entity *other;
    int length;

    // Get arguments.
    std::string character = getArgument(args);
    std::string valuestr = getArgument(args);


    // Check for a valid player.
    other = gameHandler->getCharacterByNameSlow(character);
    if (!other)
    {
        say("Invalid character", player);
        return;
    }

    // Turn the length back to an integer.
    if (valuestr.empty())
        length = Configuration::getValue("command_defaultMuteLength", 60);
    else
        length = utils::stringToInt(valuestr);

    if (length < 0)
    {
        say("Invalid length, using default", player);
        length = Configuration::getValue("command_defaultMuteLength", 60);
    }

    // Mute the player.
    other->getComponent<CharacterComponent>()->mute(length);

    const std::string &playerName =
            player->getComponent<BeingComponent>()->getName();
    const std::string &otherName =
            other->getComponent<BeingComponent>()->getName();

    // Feedback.
    std::stringstream targetMsg;
    std::stringstream userMsg;
    if (length > 0)
    {
        targetMsg << playerName << " muted you for "
        << length << " seconds.";

        userMsg << "You muted " << otherName
        << " for " << length << " seconds.";
    }
    else
    {
        targetMsg << playerName << " unmuted you.";
        userMsg << "You unmuted " << otherName << ".";
    }
    say(targetMsg.str(), other);
    say(userMsg.str(), player);

    // log transaction
    std::stringstream msg;
    if (length > 0)
    {
        msg << "User muted " << otherName << " for " << length << " seconds.";
    } else {
        msg << "User unmuted " << otherName;
    }
    int databaseId =
            player->getComponent<CharacterComponent>()->getDatabaseID();
    accountHandler->sendTransaction(databaseId, TRANS_CMD_MUTE, msg.str());
}

static void handleDie(Entity *player, std::string &)
{
    auto *hpAttribute = attributeManager->getAttributeInfo(ATTR_HP);
    player->getComponent<BeingComponent>()->setAttribute(*player, hpAttribute, 0);
    say("You've killed yourself.", player);
}

static void handleKill(Entity *player, std::string &args)
{
    Entity *other;

    // get arguments
    std::string character = getArgument(args);

    // check for valid player
    other = gameHandler->getCharacterByNameSlow(character);
    if (!other)
    {
        say("Invalid character", player);
        return;
    }

    // kill the player
    auto *hpAttribute = attributeManager->getAttributeInfo(ATTR_HP);
    other->getComponent<BeingComponent>()->setAttribute(*player, hpAttribute, 0);

    // feedback
    std::stringstream targetMsg;
    std::stringstream userMsg;
    targetMsg << "You were killed by server command from "
              << player->getComponent<BeingComponent>()->getName() << ".";
    userMsg << "You killed "
            << other->getComponent<BeingComponent>()->getName() << ".";
    say(targetMsg.str(), other);
    say(userMsg.str(), player);

    // log transaction
    std::stringstream logMsg;
    logMsg << "User killed "
           << other->getComponent<BeingComponent>()->getName();
    int databaseId =
            player->getComponent<CharacterComponent>()->getDatabaseID();
    accountHandler->sendTransaction(databaseId, TRANS_CMD_KILL, logMsg.str());
}

static void handleKick(Entity *player, std::string &args)
{
    Entity *other;

    // get arguments
    std::string character = getArgument(args);

    // check for valid player
    other = gameHandler->getCharacterByNameSlow(character);
    if (!other)
    {
        say("Invalid character", player);
        return;
    }

    // send feedback
    std::stringstream userMsg;
    userMsg << "You kicked "
            << other->getComponent<BeingComponent>()->getName() << ".";
    say(userMsg.str(), player);


    auto *characterComponent =
            player->getComponent<CharacterComponent>();

    // disconnect the client
    MessageOut msg(GPMSG_CONNECT_RESPONSE);
    msg.writeInt8(ERRMSG_ADMINISTRATIVE_LOGOFF);
    characterComponent->getClient()->disconnect(msg);

    // log transaction
    std::stringstream logMsg;
    logMsg << "User kicked "
           << other->getComponent<BeingComponent>()->getName();
    int databaseId =
            player->getComponent<CharacterComponent>()->getDatabaseID();
    accountHandler->sendTransaction(databaseId, TRANS_CMD_KICK, logMsg.str());
}


static void handleLog(Entity *player, std::string &msg)
{
    if (msg.empty())
    {
        say("Invalid number of arguments given.", player);
        say("Usage: @log <message>", player);
        return;
    }

    // log transaction
    std::string logmsg = "[silent] " + msg;
    int databaseId =
            player->getComponent<CharacterComponent>()->getDatabaseID();
    accountHandler->sendTransaction(databaseId, TRANS_CMD_LOG, logmsg);

    // send feedback
    say("Message logged", player);
}

static void handleLogsay(Entity *player, std::string &msg)
{
    if (msg.empty())
    {
        say("Invalid number of arguments given.", player);
        say("Usage: @logsay <message>", player);
        return;
    }

    GameState::sayAround(player, msg);

    // log transaction
    std::string logmsg = "[public] " + msg;
    int databaseId =
            player->getComponent<CharacterComponent>()->getDatabaseID();
    accountHandler->sendTransaction(databaseId, TRANS_CMD_LOG, logmsg);

    // send feedback
    say("Message logged", player);
}

static void handleKillMonsters(Entity *player, std::string &)
{
    const MapComposite *map = player->getMap();
    int count = 0;

    for (BeingIterator it(map->getWholeMapIterator()); it; ++it)
    {
        if ((*it)->getType() == OBJECT_MONSTER &&
            (*it)->getComponent<BeingComponent>()->getAction() != DEAD)
        {
            (*it)->getComponent<BeingComponent>()->died(**it);
            count++;
        }
    }

    std::stringstream ss;
    ss << "You killed " << count << " monster" << (count > 1 ? "s." : ".");
    say(ss.str(), player);

    // log transaction
    std::string msg = "User killed all monsters on map " + map->getName();
    int databaseId =
            player->getComponent<CharacterComponent>()->getDatabaseID();
    accountHandler->sendTransaction(databaseId, TRANS_CMD_KILLMONSTERS, msg);
}

static void handleCraft(Entity *player, std::string &args)
{
    std::stringstream errMsg;
    std::list<InventoryItem> recipe;
    Inventory playerInventory(player);

    while (true)
    {
        // parsing
        std::string strItem = getArgument(args);
        ItemClass* item = itemManager->getItemByName(strItem);
        std::string strAmount = getArgument(args);
        int amount = utils::stringToInt(strAmount);

        // syntax error checking
        if (strItem.empty())
        {
            // the item list has ended
            break;
        }
        if (!item)
        {
            // item wasn't found in the item database
            errMsg << "Unknown item: \"" << strItem << "\".";
            break;
        }

        if (strAmount.empty())
        {
            // the last item in the list has no amount defined
            errMsg << "No amount given for \"" << strItem << "\".";
            break;
        }
        if (amount < 1)
        {
            errMsg << "Illegal amount \""<< strAmount << "\" for item \"" << strItem << "\".";
            break;
        }

        // inventory checking
        int available = playerInventory.count(item->getDatabaseID());
        if (available == 0)
        {
            errMsg << "You have no "<< strItem << " in your inventory.";
            break;
        }
        if (available < amount)
        {
            errMsg << "You haven't got that many "<< strItem << "s in your inventory.";
            break;
        }

        // when there is still no break, add the item;
        InventoryItem recipeItem;
        recipeItem.itemId = item->getDatabaseID();
        recipeItem.amount = amount;
        recipe.push_back(recipeItem);
    }

    if (!errMsg.str().empty())
    {
        // when an error occured, output the error
        say(errMsg.str(), player);
        return;
    } else {
        // pass to script engine. The engine is responsible for all
        // further processing of the crafting operation, including
        // outputting an error message when the recipe is invalid.
        ScriptManager::performCraft(player, recipe);
    }
}

static void handleGetPos(Entity *player, std::string &args)
{
    std::string character = getArgument(args);
    if (character.empty())
    {
        say("Invalid amount of arguments given.", player);
        say("Usage: @getpos <character>", player);
        return;
    }
    Entity *other;
    other = gameHandler->getCharacterByNameSlow(character);
    if (!other)
    {
        say("Invalid character, or player is offline.", player);
        return;
    }
    const Point &pos = other->getComponent<ActorComponent>()->getPosition();
    std::stringstream str;
    str << "The current location of "
        << character
        << " is map "
        << other->getMap()->getID()
        << " ["
        << pos.x
        << ":"
        << pos.y
        << "]";
    say(str.str(), player);
}

static void handleEffect(Entity *player, std::string &args)
{
    std::vector<std::string> arguments;
    for (std::string arg = getArgument(args); !arg.empty();
         arg = getArgument(args))
    {
        arguments.push_back(arg);
    }

    if (arguments.size() == 1)
    {
        int id = utils::stringToInt(arguments[0]);
        Effects::show(id, player);
    }
    else if (arguments.size() == 2)
    {
        int id = utils::stringToInt(arguments[0]);
        Entity *p = gameHandler->getCharacterByNameSlow(arguments[1]);
        if (!p)
        {
            say("Invalid target player.", player);
            return;
        }
        Effects::show(id, p);
    }
    else if (arguments.size() == 3)
    {
        int id = utils::stringToInt(arguments[0]);
        int x = utils::stringToInt(arguments[1]);
        int y = utils::stringToInt(arguments[2]);
        Effects::show(id, player->getMap(), Point(x, y));
    }
    else
    {
        say("Invalid amount of arguments given.", player);
        say("Usage: @effect <effectid> <x> <y> / <effectid> <being> / "
            "<effectid>", player);
    }
}

static void handleGiveAbility(Entity *player, std::string &args)
{
    std::string character = getArgument(args);
    std::string ability = getArgument(args);
    if (character.empty() || ability.empty())
    {
        say("Invalid amount of arguments given.", player);
        say("Usage: @giveability <character> <ability>", player);
        return;
    }

    Entity *other;
    if (character == "#")
        other = player;
    else
        other = gameHandler->getCharacterByNameSlow(character);

    if (!other)
    {
        say("Invalid character, or player is offline.", player);
        return;
    }

    int abilityId;
    if (utils::isNumeric(ability))
        abilityId = utils::stringToInt(ability);
    else
        abilityId = abilityManager->getId(ability);

    if (abilityId <= 0 ||
        !other->getComponent<AbilityComponent>()->giveAbility(abilityId))
    {
        say("Invalid ability.", player);
        return;
    }
}

static void handleTakeAbility(Entity *player, std::string &args)
{
    std::string character = getArgument(args);
    std::string ability = getArgument(args);
    if (character.empty() || ability.empty())
    {
        say("Invalid amount of arguments given.", player);
        say("Usage: @takeability <character> <ability>", player);
        return;
    }

    Entity *other;
    if (character == "#")
        other = player;
    else
        other = gameHandler->getCharacterByNameSlow(character);

    if (!other)
    {
        say("Invalid character, or player is offline.", player);
        return;
    }

    int abilityId;
    if (utils::isNumeric(ability))
        abilityId = utils::stringToInt(ability);
    else
        abilityId = abilityManager->getId(ability);

    if (abilityId <= 0)
    {
        say("Invalid ability.", player);
        return;
    }
    if (!other->getComponent<AbilityComponent>()->takeAbility(abilityId))
    {
        say("Character does not have ability.", player);
        return;
    }
}

static void handleRechargeAbility(Entity *player, std::string &args)
{
    std::string character = getArgument(args);
    std::string ability = getArgument(args);
    std::string newMana = getArgument(args);
    if (character.empty() || ability.empty())
    {
        say("Invalid amount of arguments given.", player);
        say("Usage: @rechargeability <character> <ability>", player);
        return;
    }

    Entity *other;
    if (character == "#")
        other = player;
    else
        other = gameHandler->getCharacterByNameSlow(character);

    if (!other)
    {
        say("Invalid character, or player is offline.", player);
        return;
    }

    int abilityId;
    if (utils::isNumeric(ability))
        abilityId = utils::stringToInt(ability);
    else
        abilityId = abilityManager->getId(ability);

    const AbilityManager::AbilityInfo *info =
            abilityManager->getAbilityInfo(abilityId);

    if (!info)
    {
        say("Invalid ability.", player);
        return;
    }
    other->getComponent<AbilityComponent>()->setAbilityCooldown(abilityId, 0);
}

static void handleListAbility(Entity *player, std::string &args)
{
    std::string character = getArgument(args);
    if (character.empty())
    {
        say("Invalid amount of arguments given.", player);
        say("Usage: @listabilities <character>", player);
        return;
    }

    Entity *other;
    if (character == "#")
        other = player;
    else
        other = gameHandler->getCharacterByNameSlow(character);

    if (!other)
    {
        say("Invalid character, or player is offline.", player);
        return;
    }

    auto *abilityComponent = other->getComponent<AbilityComponent>();

    say("Abilities of character " +
        other->getComponent<BeingComponent>()->getName() + ":", player);
    for (auto &abilityIt : abilityComponent->getAbilities())
    {
        const AbilityValue &info = abilityIt.second;
        std::stringstream str;
        str << info.abilityInfo->id << ": " << info.abilityInfo->name;
        say(str.str(), player);
    }
}

static void handleSetAttributePoints(Entity *player, std::string &args)
{
    std::string character = getArgument(args);
    std::string attributePoints = getArgument(args);
    if (character.empty())
    {
        say("Invalid amount of arguments given.", player);
        say("Usage: @setattributepoints <character> <point>", player);
        return;
    }

    Entity *other;
    if (character == "#")
        other = player;
    else
        other = gameHandler->getCharacterByNameSlow(character);

    if (!other)
    {
        say("Invalid character, or player is offline.", player);
        return;
    }

    if (!utils::isNumeric(attributePoints))
    {
        say("Invalid character, or player is offline.", player);
        return;
    }

    auto *characterComponent = other->getComponent<CharacterComponent>();

    characterComponent->setAttributePoints(utils::stringToInt(attributePoints));
}

static void handleSetCorrectionPoints(Entity *player, std::string &args)
{
    std::string character = getArgument(args);
    std::string correctionPoints = getArgument(args);
    if (character.empty())
    {
        say("Invalid amount of arguments given.", player);
        say("Usage: @setcorrectionpoints <character> <point>", player);
        return;
    }

    Entity *other;
    if (character == "#")
        other = player;
    else
        other = gameHandler->getCharacterByNameSlow(character);

    if (!other)
    {
        say("Invalid character, or player is offline.", player);
        return;
    }

    if (!utils::isNumeric(correctionPoints))
    {
        say("Invalid character, or player is offline.", player);
        return;
    }

    auto *characterComponent = other->getComponent<CharacterComponent>();

    characterComponent->setCorrectionPoints(utils::stringToInt(correctionPoints));
}

void CommandHandler::handleCommand(Entity *player,
                                   const std::string &command)
{
    // get command type, and arguments
    // remove first character (the @)
    std::string::size_type pos = command.find(' ');
    std::string type(command, 1, pos == std::string::npos ? pos : pos - 1);
    std::string args(command, pos == std::string::npos ? command.size() : pos + 1);

    PermissionManager::Result r = PermissionManager::checkPermission(player, "@"+type);
    switch (r)
    {
    case PermissionManager::PMR_DENIED:
        say("Permissions denied!", player);
        break;
    case PermissionManager::PMR_UNKNOWN:
        say("Unknown command. Enter @help to view the list of available commands.", player);
        break;
    case PermissionManager::PMR_ALLOWED:
        // handle the command
        for (size_t i = 0; cmdRef[i].cmd != nullptr; i++)
        {
            if (cmdRef[i].cmd == type)
            {
                cmdRef[i].func(player,args);
                return;
            };
        }

        say("Command not implemented.", player);
        break;
    }
}
