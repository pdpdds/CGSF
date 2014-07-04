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

#include <iosfwd>
#include <queue>
#include <enet/enet.h>

#include "bandwidth.h"
#include "messageout.h"
#include "netcomputer.h"

#include "../utils/logger.h"
#include "../utils/processorutils.h"

NetComputer::NetComputer(ENetPeer *peer):
    mPeer(peer)
{
}

bool NetComputer::isConnected() const
{
    return (mPeer->state == ENET_PEER_STATE_CONNECTED);
}

void NetComputer::disconnect(const MessageOut &msg)
{
    if (isConnected())
    {
        /* ChannelID 0xFF is the channel used by enet_peer_disconnect.
         * If a reliable packet is send over this channel ENet guaranties
         * that the message is recieved before the disconnect request.
         */
        send(msg, ENET_PACKET_FLAG_RELIABLE, 0xFF);

        /* ENet generates a disconnect event
         * (notifying the connection handler).
         */
        enet_peer_disconnect(mPeer, 0);
    }
}

void NetComputer::send(const MessageOut &msg, bool reliable,
                       unsigned channel)
{
    LOG_DEBUG("Sending message " << msg << " to " << *this);

    gBandwidth->increaseClientOutput(this, msg.getLength());

    ENetPacket *packet;
    packet = enet_packet_create(msg.getData(),
                                msg.getLength(),
                                reliable ? ENET_PACKET_FLAG_RELIABLE : 0);

    if (packet)
    {
        enet_peer_send(mPeer, channel, packet);
    }
    else
    {
        LOG_ERROR("Failure to create packet!");
    }
}

std::ostream &operator <<(std::ostream &os, const NetComputer &comp)
{
    // address.host contains the ip-address in network-byte-order
    if (utils::processor::isLittleEndian)
        os << ( comp.mPeer->address.host & 0x000000ff)        << "."
           << ((comp.mPeer->address.host & 0x0000ff00) >> 8)  << "."
           << ((comp.mPeer->address.host & 0x00ff0000) >> 16) << "."
           << ((comp.mPeer->address.host & 0xff000000) >> 24);
    else
    // big-endian
    // TODO: test this
        os << ((comp.mPeer->address.host & 0xff000000) >> 24) << "."
           << ((comp.mPeer->address.host & 0x00ff0000) >> 16) << "."
           << ((comp.mPeer->address.host & 0x0000ff00) >> 8)  << "."
           << ((comp.mPeer->address.host & 0x000000ff));

    return os;
}

int NetComputer::getIP() const
{
    return mPeer->address.host;
}
