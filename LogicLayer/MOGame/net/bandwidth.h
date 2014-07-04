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

#ifndef BANDWIDTH_H
#define BANDWIDTH_H

#include <map>

class NetComputer;

class BandwidthMonitor
{
public:
    BandwidthMonitor();
    void increaseInterServerOutput(int size);
    void increaseInterServerInput(int size);
    void increaseClientOutput(NetComputer *nc, int size);
    void increaseClientInput(NetComputer *nc, int size);
    int totalInterServerOut() const { return mAmountServerOutput; }
    int totalInterServerIn() const { return mAmountServerInput; }
    int totalClientOut() const { return mAmountClientOutput; }
    int totalClientIn() const { return mAmountClientInput; }

private:
    int mAmountServerOutput;
    int mAmountServerInput;
    int mAmountClientOutput;
    int mAmountClientInput;
    // map of client to output and input
    typedef std::map<NetComputer*, std::pair<int, int> > ClientBandwidth;
    ClientBandwidth mClientBandwidth;
};

extern BandwidthMonitor *gBandwidth;

#endif
