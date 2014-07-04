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

#include "dataprovider.h"

#include "utils/logger.h"

namespace dal
{

PerformTransaction::PerformTransaction(DataProvider *dataProvider)
    : mDataProvider(dataProvider)
    , mTransactionStarted(false)
    , mCommitted(false)
{
    if (!mDataProvider->inTransaction())
    {
        mDataProvider->beginTransaction();
        mTransactionStarted = true;
    }
}

PerformTransaction::~PerformTransaction()
{
    if (mTransactionStarted && !mCommitted)
        mDataProvider->rollbackTransaction();
}

void PerformTransaction::commit()
{
    if (mTransactionStarted)
    {
        mDataProvider->commitTransaction();
        mCommitted = true;
        mTransactionStarted = false;
    }
}


DataProvider::DataProvider()
    throw()
        : mIsConnected(false),
          mRecordSet()
{
}

DataProvider::~DataProvider()
    throw()
{
}

/**
 * Get the connection status.
 *
 * @return true if connected.
 */
bool DataProvider::isConnected() const
    throw()
{
    return mIsConnected;
}

/**
 * Get the database name.
 */
std::string DataProvider::getDbName() const
{
    if (!isConnected())
        return std::string();

    return mDbName;
}

} // namespace dal
