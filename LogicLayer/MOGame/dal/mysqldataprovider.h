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

#ifndef MYSQL_DATA_PROVIDER_H
#define MYSQL_DATA_PROVIDER_H


#include <iosfwd>
// added to compile under windows
#ifdef WIN32
#include <winsock2.h>
#endif
#include <mysql.h>
#include <climits>

#include "dataprovider.h"
#include "common/configuration.h"
#include "utils/logger.h"

namespace dal
{

/**
 * A MySQL Data Provider.
 */
class MySqlDataProvider: public DataProvider
{
    public:
        /**
         * Replacement for mysql my_bool datatype used in mysql_autocommit()
         * function.
         */
        enum
        {
            AUTOCOMMIT_OFF = 0,
            AUTOCOMMIT_ON = 1
        };

        MySqlDataProvider()
            throw();

        ~MySqlDataProvider()
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
        const RecordSet&
        execSql(const std::string& sql,
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
        const RecordSet& processSql() ;

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
        /**
         * Bind Value (MYSQL_BIND*)
         * @param place - which parameter to bind to
         * @param value - the MySQL bind structure itself
         */
        void bindValue(int place, MYSQL_BIND* bind);

        /** Clean potential binds */
        void cleanBinds();

        /** defines the name of the hostname config parameter */
        static const std::string CFGPARAM_MYSQL_HOST;
        /** defines the name of the server port config parameter */
        static const std::string CFGPARAM_MYSQL_PORT;
        /** defines the name of the database config parameter */
        static const std::string CFGPARAM_MYSQL_DB;
        /** defines the name of the username config parameter */
        static const std::string CFGPARAM_MYSQL_USER;
        /** defines the name of the password config parameter */
        static const std::string CFGPARAM_MYSQL_PWD;

        /** defines the default value of the CFGPARAM_MYSQL_HOST parameter */
        static const std::string CFGPARAM_MYSQL_HOST_DEF;
        /** defines the default value of the CFGPARAM_MYSQL_PORT parameter */
        static const unsigned CFGPARAM_MYSQL_PORT_DEF;
        /** defines the default value of the CFGPARAM_MYSQL_DB parameter */
        static const std::string CFGPARAM_MYSQL_DB_DEF;
        /** defines the default value of the CFGPARAM_MYSQL_USER parameter */
        static const std::string CFGPARAM_MYSQL_USER_DEF;
        /** defines the default value of the CFGPARAM_MYSQL_PWD parameter */
        static const std::string CFGPARAM_MYSQL_PWD_DEF;


        /** The handle to the database connection */
        MYSQL *mDb;
        /** The prepared statement to process */
        MYSQL_STMT *mStmt;
        /** The Bind structure used in prepared statement */
        MYSQL_BIND* mBind;
        /** Tells whether we're in the middle of a transaction */
        bool mInTransaction;
};


} // namespace dal

#endif // MYSQL_DATA_PROVIDER_H
