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

#include "dataproviderfactory.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if defined (MYSQL_SUPPORT)
#include "mysqldataprovider.h"
#elif defined (POSTGRESQL_SUPPORT)
#include "pqdataprovider.h"
#elif defined (SQLITE_SUPPORT)
#include "sqlitedataprovider.h"
#else
#error "no database backend defined"
#endif

namespace dal
{

DataProviderFactory::DataProviderFactory()
    throw()
{
}

DataProviderFactory::~DataProviderFactory()
    throw()
{
}

/**
 * Create a data provider.
 */
DataProvider *DataProviderFactory::createDataProvider()
{
#if defined (MYSQL_SUPPORT)
    MySqlDataProvider* provider = new MySqlDataProvider;
    return provider;
#elif defined (POSTGRESQL_SUPPORT)
    PqDataProvider *provider = new PqDataProvider;
    return provider;
#else // SQLITE_SUPPORT
    SqLiteDataProvider* provider = new SqLiteDataProvider;
    return provider;
#endif
}

} // namespace dal
