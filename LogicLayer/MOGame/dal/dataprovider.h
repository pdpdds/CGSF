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

#ifndef DATA_PROVIDER_H
#define DATA_PROVIDER_H


#include <string>
#include <stdexcept>

#include "recordset.h"

namespace dal
{

class DataProvider;

/**
 * Enumeration type for the database backends.
 */
typedef enum {
    DB_BKEND_MYSQL,
    DB_BKEND_SQLITE,
    DB_BKEND_POSTGRESQL
} DbBackends;

/**
 * Begins a transaction on a given data provider. When the transaction is
 * complete, commit() should be called. When the destructor is called before
 * commit() is called, the transaction is rolled back.
 *
 * This avoids having to remember to call rollback in a lot of places, and
 * makes exception handling easier.
 */
class PerformTransaction
{
public:
    PerformTransaction(DataProvider *dataProvider);
    ~PerformTransaction();

    void commit();

private:
    DataProvider *mDataProvider;
    bool mTransactionStarted;
    bool mCommitted;
};

/**
 * An abstract data provider.
 *
 * Notes:
 *     - depending on the database backend, the connection to an unexisting
 *       database may actually create it as a side-effect (e.g. SQLite).
 *
 * Limitations:
 *     - this class does not provide APIs for:
 *         - remote connections,
 *         - creating new databases,
 *         - dropping existing databases.
 */
class DataProvider
{
    public:
        DataProvider()
            throw();

        virtual ~DataProvider()
            throw();

        bool isConnected() const
            throw();

        /**
         * Get the name of the database backend.
         *
         * @return the database backend name.
         */
        virtual DbBackends getDbBackend() const
            throw() = 0;

        /**
         * Create a connection to the database.
         *
         * Each dataprovider is responsible to have default values and load
         * necessary options from the config file.
         *
         * @exception DbConnectionFailure if unsuccessful connection.
         */
        virtual void connect() = 0;


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
        virtual const RecordSet&
        execSql(const std::string& sql,
                const bool refresh = false) = 0;


        /**
         * Close the connection to the database.
         *
         * @exception DbDisconnectionFailure if unsuccessful disconnection.
         */
        virtual void disconnect() = 0;

        std::string getDbName() const;

        /**
         * Starts a transaction.
         *
         * @exception std::runtime_error if a transaction is still open
         */
        virtual void beginTransaction()
            throw (std::runtime_error) = 0;

        /**
         * Commits a transaction.
         *
         * @exception std::runtime_error if no connection is currently open.
         */
        virtual void commitTransaction()
            throw (std::runtime_error) = 0;

        /**
         * Rollback a transaction.
         *
         * @exception std::runtime_error if no connection is currently open.
         */
        virtual void rollbackTransaction()
            throw (std::runtime_error) = 0;

        /**
         * Returns whether the data provider is currently in a transaction.
         */
        virtual bool inTransaction() const = 0;

        /**
         * Returns the number of changed rows by the last executed SQL
         * statement.
         *
         * @return Number of rows that have changed.
         */
        virtual unsigned getModifiedRows() const = 0;

        /**
         * Returns the last inserted value of an autoincrement column after an
         * INSERT statement.
         *
         * @return last autoincrement value.
         */
        virtual unsigned getLastId() const = 0;

        /**
         * Prepare SQL statement
         */
        virtual bool prepareSql(const std::string &sql) = 0;

        /**
         * Process SQL statement
         * SQL statement needs to be prepared and parameters binded before
         * calling this function
         */
        virtual const RecordSet& processSql() = 0;

        /**
         * Bind Value (String)
         * @param place - which parameter to bind to
         * @param value - the string to bind
         */
        virtual void bindValue(int place, const std::string &value) = 0;

        /**
         * Bind Value (Integer)
         * @param place - which parameter to bind to
         * @param value - the integer to bind
         */
        virtual void bindValue(int place, int value) = 0;

    protected:
        std::string mDbName;  /**< the database name */
        bool mIsConnected;    /**< the connection status */
        std::string mSql;     /**< cache the last SQL query */
        RecordSet mRecordSet; /**< cache the result of the last SQL query */
};


} // namespace dal

#endif // DATA_PROVIDER_H
