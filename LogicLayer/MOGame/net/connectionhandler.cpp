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

#include <algorithm>

#include "net/connectionhandler.h"

#include "common/configuration.h"
#include "net/bandwidth.h"
#include "net/messagein.h"
#include "net/messageout.h"
#include "net/netcomputer.h"
#include "utils/logger.h"

#ifdef ENET_VERSION_CREATE
#define ENET_CUTOFF ENET_VERSION_CREATE(1,3,0)
#else
#define ENET_CUTOFF 0xFFFFFFFF
#endif

bool ConnectionHandler::startListen(enet_uint16 port,
                                    const std::string &listenHost)
{
    // Bind the server to the default localhost.
    address.host = ENET_HOST_ANY;
    address.port = port;

    if (!listenHost.empty())
        enet_address_set_host(&address, listenHost.c_str());

    LOG_INFO("Listening on port " << port << "...");
#if defined(ENET_VERSION) && ENET_VERSION >= ENET_CUTOFF
    host = enet_host_create(
            &address    /* the address to bind the server host to */,
            Configuration::getValue("net_maxClients", 1000) /* allowed connections */,
            0           /* unlimited channel count */,
            0           /* assume any amount of incoming bandwidth */,
            0           /* assume any amount of outgoing bandwidth */);
#else
    host = enet_host_create(
            &address    /* the address to bind the server host to */,
            Configuration::getValue("net_maxClients", 1000) /* allowed connections */,
            0           /* assume any amount of incoming bandwidth */,
            0           /* assume any amount of outgoing bandwidth */);
#endif

    return host != 0;
}

void ConnectionHandler::stopListen()
{
    // - Disconnect all clients (close sockets)

    // TODO: probably there's a better way.
    ENetPeer *currentPeer;

    for (currentPeer = host->peers;
         currentPeer < &host->peers[host->peerCount];
         ++currentPeer)
    {
        if (currentPeer->state == ENET_PEER_STATE_CONNECTED)
        {
            enet_peer_disconnect(currentPeer, 0);
            enet_host_flush(host);
            enet_peer_reset(currentPeer);
        }
    }
    enet_host_destroy(host);
    // FIXME: memory leak on NetComputers
}

void ConnectionHandler::flush()
{
    enet_host_flush(host);
}

void ConnectionHandler::process(enet_uint32 timeout)
{
    ENetEvent event;
    // Process Enet events and do not block.
    while (enet_host_service(host, &event, timeout) > 0) {
        switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
            {
                NetComputer *comp = computerConnected(event.peer);
                clients.push_back(comp);
                LOG_INFO("A new client connected from " << *comp << ":"
                         << event.peer->address.port << " to port "
                         << host->address.port);

                // Store any relevant client information here.
                event.peer->data = (void *)comp;
            } break;

            case ENET_EVENT_TYPE_RECEIVE:
            {
                NetComputer *comp =
                    static_cast<NetComputer*>(event.peer->data);

                // If the scripting subsystem didn't hook the message
                // it will be handled by the default message handler.

                // Make sure that the packet is big enough (> short)
                if (event.packet->dataLength >= 2) {
                    MessageIn msg((char *)event.packet->data,
                                  event.packet->dataLength);
                    LOG_DEBUG("Received message " << msg << " from "
                              << *comp);

                    gBandwidth->increaseClientInput(comp, event.packet->dataLength);

                    processMessage(comp, msg);
                } else {
                    LOG_ERROR("Message too short from " << *comp);
                }

                /* Clean up the packet now that we're done using it. */
                enet_packet_destroy(event.packet);
            } break;

            case ENET_EVENT_TYPE_DISCONNECT:
            {
                NetComputer *comp =
                    static_cast<NetComputer*>(event.peer->data);

                LOG_INFO("" << *comp << " disconnected.");

                // Reset the peer's client information.
                computerDisconnected(comp);
                clients.erase(std::find(clients.begin(), clients.end(), comp));
                event.peer->data = nullptr;
            } break;

            default: break;
        }
    }
}

void ConnectionHandler::sendToEveryone(const MessageOut &msg)
{
    for (NetComputers::iterator i = clients.begin(), i_end = clients.end();
         i != i_end; ++i)
    {
        (*i)->send(msg);
    }
}

unsigned ConnectionHandler::getClientCount() const
{
    return clients.size();
}
