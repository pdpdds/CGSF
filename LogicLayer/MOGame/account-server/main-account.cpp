/*
 *  The Mana Server
 *  Copyright (C) 2004-2010  The Mana World Development Team
 *  Copyright (C) 2010  The Mana Developers
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

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#ifdef __MINGW32__
#include "common/winver.h"
#endif

#include "account-server/accounthandler.h"
#include "account-server/serverhandler.h"
#include "account-server/storage.h"
#include "chat-server/chatchannelmanager.h"
#include "chat-server/chathandler.h"
#include "chat-server/guildmanager.h"
#include "chat-server/post.h"
#include "common/configuration.h"
#include "common/defines.h"
#include "common/manaserv_protocol.h"
#include "common/resourcemanager.h"
#include "net/bandwidth.h"
#include "net/connectionhandler.h"
#include "net/messageout.h"
#include "utils/logger.h"
#include "utils/processorutils.h"
#include "utils/stringfilter.h"
#include "utils/time.h"
#include "utils/timer.h"

#include <cstdlib>
#include <getopt.h>
#include <signal.h>
#include <iostream>
#include <fstream>
#include <physfs.h>
#include <enet/enet.h>

using utils::Logger;

#pragma comment(lib, "enet.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "lua.lib")
#pragma comment(lib, "mogame.lib")
#pragma comment(lib, "lua.lib")
#pragma comment(lib, "physfs.lib")
#pragma comment(lib, "libxml2.lib")
#pragma comment(lib, "sigc-vc120-d-2_0.lib")
#pragma comment(lib, "zlib.lib")
#pragma comment(lib, "libmysql.lib")


// Default options that automake should be able to override.
#define DEFAULT_LOG_FILE          "manaserv-account.log"
#define DEFAULT_STATS_FILE        "manaserv.stats"
#define DEFAULT_ATTRIBUTEDB_FILE  "attributes.xml"

static bool running = true;        /**< Determines if server keeps running */

utils::StringFilter *stringFilter; /**< Slang's Filter */

static std::string statisticsFile = std::string();

/** Database handler. */
Storage *storage;

/** Communications (chat) message handler */
ChatHandler *chatHandler;

ChatChannelManager *chatChannelManager;
GuildManager *guildManager;
PostManager *postalManager;
BandwidthMonitor *gBandwidth;

/** Callback used when SIGQUIT signal is received. */
static void closeGracefully(int)
{
    running = false;
}

/**
 * Initializes the server.
 */
static void initialize()
{
//20140621
	/*
    // Used to close via process signals
#if (defined __USE_UNIX98 || defined __FreeBSD__)
    signal(SIGQUIT, closeGracefully);
#endif
    signal(SIGINT, closeGracefully);
    signal(SIGTERM, closeGracefully);
*/
    std::string logFile = Configuration::getValue("log_accountServerFile",
                                                  DEFAULT_LOG_FILE);

    // Initialize PhysicsFS
    PHYSFS_init("");

    Logger::initialize(logFile);

    // Indicate in which file the statistics are put.
    statisticsFile = Configuration::getValue("log_statisticsFile",
                                             DEFAULT_STATS_FILE);

    LOG_INFO("Using statistics file: " << statisticsFile);

    ResourceManager::initialize();

    // Open database
    try
    {
        storage = new Storage;
        storage->open();
    }
    catch (std::string &error)
    {
        LOG_FATAL("Error opening the database: " << error);
        exit(EXIT_DB_EXCEPTION);
    }

    // --- Initialize the managers
    stringFilter = new utils::StringFilter;  // The slang's and double quotes filter.
    chatChannelManager = new ChatChannelManager;
    guildManager = new GuildManager;
    postalManager = new PostManager;
    gBandwidth = new BandwidthMonitor;

    // --- Initialize the global handlers
    // FIXME: Make the global handlers global vars or part of a bigger
    // singleton or a local variable in the event-loop
    chatHandler = new ChatHandler;

    // --- Initialize enet.
    if (enet_initialize() != 0)
    {
        LOG_FATAL("An error occurred while initializing ENet");
        exit(EXIT_NET_EXCEPTION);
    }

    // Initialize the processor utility functions
    utils::processor::init();

    // Seed the random number generator
    std::srand( time(nullptr) );
}


/**
 * Deinitializes the server.
 */
static void deinitializeServer()
{
    // Write configuration file
    Configuration::deinitialize();

    // Destroy message handlers.
    AccountClientHandler::deinitialize();
    GameServerHandler::deinitialize();

    // Quit ENet
    enet_deinitialize();

    delete chatHandler;

    // Destroy Managers
    delete stringFilter;
    delete chatChannelManager;
    delete guildManager;
    delete postalManager;
    delete gBandwidth;

    // Get rid of persistent data storage
    delete storage;

    PHYSFS_deinit();
}

/**
 * Dumps statistics.
 */
static void dumpStatistics(std::string accountAddress, int accountClientPort,
                           int accountGamePort, int chatClientPort)
{
    std::ofstream os(statisticsFile.c_str());
    os << "<statistics>\n";
    // Print last heartbeat
    os << "<heartbeat=\"" << utils::getCurrentDate() << "_"
    << utils::getCurrentTime() << "\" />\n";
    // Add account server information
    os << "<accountserver address=\"" << accountAddress << "\" clientport=\""
    << accountClientPort << "\" gameport=\"" << accountGamePort
    << "\" chatclientport=\"" << chatClientPort << "\" />\n";
    // Add game servers information
    GameServerHandler::dumpStatistics(os);
    os << "</statistics>\n";
}

/**
 * Show command line arguments
 */
static void printHelp()
{
    std::cout << "manaserv" << std::endl << std::endl
              << "Options: " << std::endl
              << "  -h --help          : Display this help" << std::endl
              << "     --config <path> : Set the config path to use."
              << " (Default: ./manaserv.xml)" << std::endl
              << "  -v --verbosity <n> : Set the verbosity level" << std::endl
              << "                        - 0. Fatal Errors only." << std::endl
              << "                        - 1. All Errors." << std::endl
              << "                        - 2. Plus warnings." << std::endl
              << "                        - 3. Plus standard information." << std::endl
              << "                        - 4. Plus debugging information." << std::endl
              << "     --port <n>      : Set the default port to listen on" << std::endl;
    exit(EXIT_NORMAL);
}

struct CommandLineOptions
{
    CommandLineOptions():
        verbosity(Logger::Warn),
        verbosityChanged(false),
        port(DEFAULT_SERVER_PORT),
        portChanged(false)
    {}

    std::string configPath;

    Logger::Level verbosity;
    bool verbosityChanged;

    int port;
    bool portChanged;
};

/**
 * Parse the command line arguments
 */
static void parseOptions(int argc, char *argv[], CommandLineOptions &options)
{
    const char *optString = "hv:";

    const struct option longOptions[] =
    {
        { "help",       no_argument,       0, 'h' },
        { "config",     required_argument, 0, 'c' },
        { "verbosity",  required_argument, 0, 'v' },
        { "port",       required_argument, 0, 'p' },
        { 0, 0, 0, 0 }
    };

    while (optind < argc)
    {
        //int result = getopt_long(argc, argv, optString, longOptions, nullptr);

		int result = -1;
        if (result == -1)
            break;

        switch (result)
        {
            default: // Unknown option.
            case 'h':
                // Print help.
                printHelp();
                break;
            case 'c':
                // Change config filename and path.
                options.configPath = optarg;
                break;
            case 'v':
                options.verbosity = static_cast<Logger::Level>(atoi(optarg));
                options.verbosityChanged = true;
                LOG_INFO("Using log verbosity level " << options.verbosity);
                break;
            case 'p':
                options.port = atoi(optarg);
                options.portChanged = true;
                break;
        }
    }
}


/**
 * Main function, initializes and runs server.
 */
int main(int argc, char *argv[])
{
    // Parse command line options
    CommandLineOptions options;
//20140621
    //parseOptions(argc, argv, options);

    if (!Configuration::initialize(options.configPath))
    {
        LOG_FATAL("Refusing to run without configuration!");
        exit(EXIT_CONFIG_NOT_FOUND);
    }

    // Check inter-server password.
    if (Configuration::getValue("net_password", std::string()).empty())
    {
        LOG_FATAL("SECURITY WARNING: 'net_password' not set!");
        exit(EXIT_BAD_CONFIG_PARAMETER);
    }

    // General initialization
    initialize();

#ifdef PACKAGE_VERSION
    LOG_INFO("The Mana Account+Chat Server v" << PACKAGE_VERSION);
#else
    LOG_INFO("The Mana Account+Chat Server (unknown version)");
#endif
    LOG_INFO("Manaserv Protocol version " << ManaServ::PROTOCOL_VERSION
             << ", Enet version " << ENET_VERSION_MAJOR << "."
             << ENET_VERSION_MINOR << "." << ENET_VERSION_PATCH
             << ", Database version " << ManaServ::SUPPORTED_DB_VERSION);

    if (!options.verbosityChanged)
        options.verbosity = static_cast<Logger::Level>(
                            Configuration::getValue("log_accountServerLogLevel",
                                                    options.verbosity) );
    Logger::setVerbosity(options.verbosity);

    std::string accountHost = Configuration::getValue("net_accountHost",
                                                      "localhost");

    // We separate the chat host as the chat server will be separated out
    // from the account server.
    std::string chatHost = Configuration::getValue("net_chatHost",
                                                   "localhost");

    // Setting the listen ports
    // Note: The accountToGame and chatToClient listen ports auto offset
    // to accountToClient listen port when they're not set,
    // or to DEFAULT_SERVER_PORT otherwise.
    if (!options.portChanged)
        options.port = Configuration::getValue("net_accountListenToClientPort",
                                               options.port);
    int accountGamePort = Configuration::getValue("net_accountListenToGamePort",
                                                  options.port + 1);
    int chatClientPort = Configuration::getValue("net_chatListenToClientPort",
                                                 options.port + 2);

    bool debugNetwork = Configuration::getBoolValue("net_debugMode", false);
    MessageOut::setDebugModeEnabled(debugNetwork);

    if (!AccountClientHandler::initialize(DEFAULT_ATTRIBUTEDB_FILE,
                                          options.port, accountHost) ||
        !GameServerHandler::initialize(accountGamePort, accountHost) ||
        !chatHandler->startListen(chatClientPort, chatHost))
    {
        LOG_FATAL("Unable to create an ENet server host.");
        return EXIT_NET_EXCEPTION;
    }

    // Dump statistics every 10 seconds.
    utils::Timer statTimer(10000);
    // Check for expired bans every 30 seconds
    utils::Timer banTimer(30000);

    statTimer.start();
    banTimer.start();

    // Write startup time to database as system world state variable
    std::stringstream timestamp;
    timestamp << time(nullptr);
    storage->setWorldStateVar("accountserver_startup", timestamp.str(),
                              Storage::SystemMap);

    while (running)
    {
        AccountClientHandler::process();
        GameServerHandler::process();
        chatHandler->process(50);

        if (statTimer.poll())
            dumpStatistics(accountHost, options.port, accountGamePort,
                           chatClientPort);

        if (banTimer.poll())
            storage->checkBannedAccounts();
    }

    LOG_INFO("Received: Quit signal, closing down...");
    chatHandler->stopListen();
    deinitializeServer();

    return EXIT_NORMAL;
}
