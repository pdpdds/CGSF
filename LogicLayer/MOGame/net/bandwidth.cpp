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

#include "bandwidth.h"

#include "netcomputer.h"

BandwidthMonitor::BandwidthMonitor():
    mAmountServerOutput(0),
    mAmountServerInput(0),
    mAmountClientOutput(0),
    mAmountClientInput(0)
{
}

void BandwidthMonitor::increaseInterServerOutput(int size)
{
    mAmountServerOutput += size;
}

void BandwidthMonitor::increaseInterServerInput(int size)
{
    mAmountServerInput += size;
}

void BandwidthMonitor::increaseClientOutput(NetComputer *nc, int size)
{
    mAmountClientOutput += size;
    // look for an existing client stored
    ClientBandwidth::iterator itr = mClientBandwidth.find(nc);

    // if there isnt one, create one
    if (itr == mClientBandwidth.end())
    {
        std::pair<ClientBandwidth::iterator, bool> retItr;
        retItr = mClientBandwidth.insert(std::pair<NetComputer*, std::pair<int, int> >(nc, std::pair<int, int>(0, 0)));
        itr = retItr.first;
    }

    itr->second.first += size;

}

void BandwidthMonitor::increaseClientInput(NetComputer *nc, int size)
{
    mAmountClientInput += size;

    // look for an existing client stored
    ClientBandwidth::iterator itr = mClientBandwidth.find(nc);

    // if there isnt one, create it
    if (itr == mClientBandwidth.end())
    {
        std::pair<ClientBandwidth::iterator, bool> retItr;
        retItr = mClientBandwidth.insert(std::pair<NetComputer*, std::pair<int, int> >(nc, std::pair<int, int>(0, 0)));
        itr = retItr.first;
    }

    itr->second.second += size;
}

