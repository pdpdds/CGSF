/*
 *  The Mana Server
 *  Copyright (C) 2006-2010  The Mana World Development Team
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

#include "net/connection.h"
#include "net/bandwidth.h"
#include "net/messagein.h"
#include "net/messageout.h"
#include "utils/logger.h"

#ifdef ENET_VERSION_CREATE
#define ENET_CUTOFF ENET_VERSION_CREATE(1,3,0)
#else
#define ENET_CUTOFF 0xFFFFFFFF
#endif

Connection::Connection():
    mRemote(0),
    mLocal(0)
{
}

bool Connection::start(const std::string &address, int port)
{
    ENetAddress enetAddress;
    enet_address_set_host(&enetAddress, address.c_str());
    enetAddress.port = port;

#if defined(ENET_VERSION) && ENET_VERSION >= ENET_CUTOFF
    mLocal = enet_host_create(nullptr /* create a client host */,
                              1 /* allow one outgoing connection */,
                              0           /* unlimited channel count */,
                              0 /* assume any amount of incoming bandwidth */,
                              0 /* assume any amount of outgoing bandwidth */);
#else
    mLocal = enet_host_create(nullptr /* create a client host */,
                              1 /* allow one outgoing connection */,
                              0 /* assume any amount of incoming bandwidth */,
                              0 /* assume any amount of outgoing bandwidth */);
#endif

    if (!mLocal)
        return false;

    // Initiate the connection, allocating channel 0.
#if defined(ENET_VERSION) && ENET_VERSION >= ENET_CUTOFF
    mRemote = enet_host_connect(mLocal, &enetAddress, 1, 0);
#else
    mRemote = enet_host_connect(mLocal, &enetAddress, 1);
#endif

    ENetEvent event;
    if (enet_host_service(mLocal, &event, 10000) <= 0 ||
        event.type != ENET_EVENT_TYPE_CONNECT)
    {
        stop();
        return false;
    }
    return mRemote;
}

void Connection::stop()
{
    if (mRemote)
        enet_peer_disconnect(mRemote, 0);
    if (mLocal)
        enet_host_flush(mLocal);
    if (mRemote)
        enet_peer_reset(mRemote);
    if (mLocal)
        enet_host_destroy(mLocal);

    mRemote = 0;
    mLocal = 0;
}

bool Connection::isConnected() const
{
    return mRemote && mRemote->state == ENET_PEER_STATE_CONNECTED;
}

void Connection::send(const MessageOut &msg, bool reliable, unsigned channel)
{
    if (!mRemote) {
        LOG_WARN("Can't send message to unconnected host! (" << msg << ")");
        return;
    }

    gBandwidth->increaseInterServerOutput(msg.getLength());

    ENetPacket *packet;
    packet = enet_packet_create(msg.getData(),
                                msg.getLength(),
                                reliable ? ENET_PACKET_FLAG_RELIABLE : 0);

    if (packet)
        enet_peer_send(mRemote, channel, packet);
    else
        LOG_ERROR("Failure to create packet!");
}

void Connection::process()
{
    ENetEvent event;
    // Process Enet events and do not block.
    while (enet_host_service(mLocal, &event, 0) > 0)
    {
        switch (event.type)
        {
            case ENET_EVENT_TYPE_RECEIVE:
                if (event.packet->dataLength >= 2)
                {
                    MessageIn msg((char *)event.packet->data,
                                  event.packet->dataLength);
                    gBandwidth->increaseInterServerInput(event.packet->dataLength);
                    processMessage(msg);
                }
                else
                {
                    LOG_WARN("Message too short.");
                }
                // Clean up the packet now that we are done using it.
                enet_packet_destroy(event.packet);
                break;

            default:
                break;
        }
    }
}
