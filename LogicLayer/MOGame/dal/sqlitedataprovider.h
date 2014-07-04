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

#ifndef SQLITE_DATA_PROVIDER_H
#define SQLITE_DATA_PROVIDER_H

#include "dataprovider.h"

#include <iosfwd>
#include <sqlite3.h>

namespace dal
{

/**
 * A SQLite Data Provider.
 */
class SqLiteDataProvider: public DataProvider
{
    public:
        SqLiteDataProvider()
            throw();

        ~SqLiteDataProvider()
            throw();

        /**
         * Get the name of the database backend.
         *
         * @return the database backend name.
         */
        DbBackends getDbBackend() const
            throw();

        /**
         * Create a connection to the database.
         *
         * @exception DbConnectionFailure if unsuccessful connection.
         */
        void connect();

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

        /**
         * Starts a transaction.
         *
         * @exception std::runtime_error if a transaction is still open
         */
        void beginTransaction()
            throw (std::runtime_error);

        /**
         * Commits a transaction.
         *
         * @exception std::runtime_error if no connection is currently open.
         */
        void commitTransaction()
            throw (std::runtime_error);

        /**
         * Rollback a transaction.
         *
         * @exception std::runtime_error if no connection is currently open.
         */
        void rollbackTransaction()
            throw (std::runtime_error);

        /**
         * Returns wheter the connection has a open transaction or is in auto-
         * commit mode.
         *
         * @return true, if a transaction is open.
         */
        bool inTransaction() const;

        /**
         * Returns the number of changed rows by the last executed SQL
         * statement.
         *
         * @return Number of rows that have changed.
         */
        unsigned getModifiedRows() const;

        /**
         * Returns the last inserted value of an autoincrement column after an
         * INSERT statement.
         *
         * @return last autoincrement value.
         */
        unsigned getLastId() const;

        /**
         * Prepare SQL statement
         */
        bool prepareSql(const std::string &sql);

        /**
         * Process SQL statement
         * SQL statement needs to be prepared and parameters binded before
         * calling this function
         */
        const RecordSet& processSql();

        /**
         * Bind Value (String)
         * @param place - which parameter to bind to
         * @param value - the string to bind
         */
        void bindValue(int place, const std::string &value);

        /**
         * Bind Value (Integer)
         * @param place - which parameter to bind to
         * @param value - the integer to bind
         */
        void bindValue(int place, int value);

    private:
        /** defines the name of the database config parameter */
        static const std::string CFGPARAM_SQLITE_DB;
        /** defines the default value of the CFGPARAM_SQLITE_DB parameter */
        static const std::string CFGPARAM_SQLITE_DB_DEF;

        sqlite3 *mDb; /**< the handle to the database connection */
        sqlite3_stmt *mStmt; /**< the prepared statement to process */
};


} // namespace dal

#endif // SQLITE_DATA_PROVIDER_H
