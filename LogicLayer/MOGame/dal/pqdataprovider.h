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

#ifndef PQDATAPROVIDER_H
#define PQDATAPROVIDER_H

#include <iosfwd>
#include <libpq-fe.h>

#include "dataprovider.h"

namespace dal
{

/**
 * A PostgreSQL Data Provider.
 */
class PqDataProvider: public DataProvider
{
    public:
        PqDataProvider()
            throw();

        ~PqDataProvider()
            throw();

        /**
         * Get name of the database backend
         *
         * @return the database backend name
         */
        DbBackends getDbBackend() const
            throw();

        /**
         * Create a connection to the database.
         *
         * @param dbName the database name.
         * @param userName the user name.
         * @param password the user password.
         *
         * @exception DbConnectionFailure if unsuccessful connection.
         */
        void connect(const std::string& dbName,
                     const std::string& userName,
                     const std::string& password);

        /**
         * Execute a SQL query.
         *
         * @param sql the SQL query.
         * @param refresh if true, refresh the cache (default = false).
         *
         * @return a recordset.
         *
         * @exception DbSqlQueryExecFailure if unsuccessful execution.
         * @exception std::runtime_error if trying to query a closed database.
         */
        const RecordSet &execSql(const std::string& sql,
                                 const bool refresh = false);

        /**
         * Close the connection to the database.
         *
         * @exception DbDisconnectionFailure if unsuccessful disconnection.
         */
        void disconnect();

    private:
        PGconn *mDb; /**<  Database connection handle */
};


} // namespace dal

#endif // PQDATAPROVIDER_H
