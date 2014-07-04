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

#include "sqlitedataprovider.h"

#include "dalexcept.h"

#include "common/configuration.h"
#include "utils/logger.h"

#include <stdexcept>
#include <limits.h>

// sqlite3_int64 is the preferred new datatype for 64-bit int values.
// see: http://www.sqlite.org/capi3ref.html#sqlite3_int64
#ifndef sqlite3_int64
typedef sqlite_int64 sqlite3_int64;
#endif

namespace dal
{

const std::string SqLiteDataProvider::CFGPARAM_SQLITE_DB     = "sqlite_database";
const std::string SqLiteDataProvider::CFGPARAM_SQLITE_DB_DEF = "mana.db";

SqLiteDataProvider::SqLiteDataProvider()
    throw()
        : mDb(0)
        , mStmt(0)
{
}

SqLiteDataProvider::~SqLiteDataProvider()
    throw()
{
    try
    {
        // make sure that the database is closed.
        // disconnect() calls sqlite3_close() which takes care of freeing
        // the memory allocated for the handle.
        if (mIsConnected)
            disconnect();
    }
    catch (...)
    {
        // ignore
    }
}

/**
 * Get the name of the database backend.
 */
DbBackends SqLiteDataProvider::getDbBackend() const
    throw()
{
    return DB_BKEND_SQLITE;
}

/**
 * Create a connection to the database.
 */
void SqLiteDataProvider::connect()
{
    // get configuration parameter for sqlite
    const std::string dbName
        = Configuration::getValue(CFGPARAM_SQLITE_DB, CFGPARAM_SQLITE_DB_DEF);

    LOG_INFO("Trying to connect with SQLite database file '"
        << dbName << "'");

    // sqlite3_open creates the database file if it does not exist
    // as a side-effect.
    if (sqlite3_open(dbName.c_str(), &mDb) != SQLITE_OK)
    {
        // save the error message thrown by sqlite3_open()
        // as we may lose it when sqlite3_close() runs.
        std::string msg(sqlite3_errmsg(mDb));

        // the SQLite3 documentation suggests that we try to close
        // the database after an unsuccessful call to sqlite3_open().
        sqlite3_close(mDb);

        // FIXME
        // 21-Jun-2005: although we did invoke sqlite3_close(), there
        // seems to be still a leak of 136 bytes here.

        throw DbConnectionFailure(msg);
    }

    // Wait up to one second when the database is busy. This should make sure
    // transaction failures due to locked databases are very rare.
    sqlite3_busy_timeout(mDb, 1000);

    // Save the Db Name.
    mDbName = dbName;

    mIsConnected = true;
    LOG_INFO("Connection to database successful.");
}

/**
 * Execute a SQL query.
 */
const RecordSet &SqLiteDataProvider::execSql(const std::string& sql,
                                             const bool refresh)
{
    if (!mIsConnected)
        throw std::runtime_error("not connected to database");

    LOG_DEBUG("Performing SQL query: " << sql);

    // do something only if the query is different from the previous
    // or if the cache must be refreshed
    // otherwise just return the recordset from cache.
    if (refresh || (sql != mSql))
    {
        char** result;
        int nRows;
        int nCols;
        char* errMsg;

        mRecordSet.clear();

        int errCode = sqlite3_get_table(
                          mDb,          // an open database
                          sql.c_str(),  // SQL to be executed
                          &result,      // result of the query
                          &nRows,       // number of result rows
                          &nCols,       // number of result columns
                          &errMsg       // error msg
                      );

        if (errCode != SQLITE_OK)
        {
            std::string msg(sqlite3_errmsg(mDb));

            LOG_ERROR("Error in SQL: " << sql << "\n" << msg);

            // free memory
            sqlite3_free_table(result);
            sqlite3_free(errMsg);

            throw DbSqlQueryExecFailure(msg);
        }

        // the first row of result[] contains the field names.
        Row fieldNames;
        for (int col = 0; col < nCols; ++col)
            fieldNames.push_back(result[col]);

        mRecordSet.setColumnHeaders(fieldNames);

        // populate the RecordSet
        for (int row = 0; row < nRows; ++row)
        {
            Row r;

            for (int col = 0; col < nCols; ++col)
                r.push_back(result[nCols + (row * nCols) + col]);

            mRecordSet.add(r);
        }

        // free memory
        sqlite3_free_table(result);
        sqlite3_free(errMsg);
    }

    return mRecordSet;
}

/**
 * Close the connection to the database.
 */
void SqLiteDataProvider::disconnect()
{
    if (!isConnected())
        return;

    // sqlite3_close() closes the connection and deallocates the connection
    // handle.
    if (sqlite3_close(mDb) != SQLITE_OK)
        throw DbDisconnectionFailure(sqlite3_errmsg(mDb));

    mDb = 0;
    mIsConnected = false;
}

void SqLiteDataProvider::beginTransaction()
    throw (std::runtime_error)
{
    if (!mIsConnected)
    {
        const std::string error = "Trying to begin a transaction while not "
            "connected to the database!";
        LOG_ERROR(error);
        throw std::runtime_error(error);
    }

    if (inTransaction())
    {
        const std::string error = "Trying to begin a transaction while anoter "
            "one is still open!";
        LOG_ERROR(error);
        throw std::runtime_error(error);
    }

    // trying to open a transaction
    try
    {
        execSql("BEGIN TRANSACTION;");
        LOG_DEBUG("SQL: started transaction");
    }
    catch (const DbSqlQueryExecFailure &e)
    {
        std::ostringstream error;
        error << "SQL ERROR while trying to start a transaction: " << e.what();
//        LOG_ERROR(error);
        throw std::runtime_error(error.str());
    }
}

void SqLiteDataProvider::commitTransaction()
    throw (std::runtime_error)
{
    if (!mIsConnected)
    {
        const std::string error = "Trying to commit a transaction while not "
            "connected to the database!";
        LOG_ERROR(error);
        throw std::runtime_error(error);
    }

    if (!inTransaction())
    {
        const std::string error = "Trying to commit a transaction while no "
            "one is open!";
        LOG_ERROR(error);
        throw std::runtime_error(error);
    }

    // trying to commit a transaction
    try
    {
        execSql("COMMIT TRANSACTION;");
        LOG_DEBUG("SQL: commited transaction");
    }
    catch (const DbSqlQueryExecFailure &e)
    {
        std::ostringstream error;
        error << "SQL ERROR while trying to commit a transaction: " << e.what();
//        LOG_ERROR(error);
        throw std::runtime_error(error.str());
    }
}

void SqLiteDataProvider::rollbackTransaction()
    throw (std::runtime_error)
{
    if (!mIsConnected)
    {
        const std::string error = "Trying to rollback a transaction while not "
            "connected to the database!";
        LOG_ERROR(error);
        throw std::runtime_error(error);
    }

    if (!inTransaction())
    {
        const std::string error = "Trying to rollback a transaction while no "
            "one is open!";
        LOG_ERROR(error);
        throw std::runtime_error(error);
    }

    // trying to rollback a transaction
    try
    {
        execSql("ROLLBACK TRANSACTION;");
        LOG_DEBUG("SQL: transaction rolled back");
    }
    catch (const DbSqlQueryExecFailure &e)
    {
        std::ostringstream error;
        error << "SQL ERROR while trying to rollback a transaction: " << e.what();
//        LOG_ERROR(error);
        throw std::runtime_error(error.str());
    }
}

unsigned SqLiteDataProvider::getModifiedRows() const
{
    if (!mIsConnected)
    {
        const std::string error = "Trying to getModifiedRows while not "
            "connected to the database!";
        LOG_ERROR(error);
        throw std::runtime_error(error);
    }

    return (unsigned) sqlite3_changes(mDb);
}

bool SqLiteDataProvider::inTransaction() const
{
    if (!mIsConnected)
    {
        const std::string error = "not connected to the database!";
        LOG_ERROR(error);
        throw std::runtime_error(error);
    }

    // The sqlite3_get_autocommit() interface returns non-zero or zero if the
    // given database connection is or is not in autocommit mode, respectively.
    // Autocommit mode is on by default. Autocommit mode is disabled by a BEGIN
    // statement. Autocommit mode is re-enabled by a COMMIT or ROLLBACK.
    const int ret = sqlite3_get_autocommit(mDb);
    return ret == 0;
}

unsigned SqLiteDataProvider::getLastId() const
{
    if (!mIsConnected)
    {
        const std::string error = "not connected to the database!";
        LOG_ERROR(error);
        throw std::runtime_error(error);
    }

    // FIXME: not sure if this is correct to bring 64bit int into int?
    const sqlite3_int64 lastId = sqlite3_last_insert_rowid(mDb);
    if (lastId > UINT_MAX)
        throw std::runtime_error("SqLiteDataProvider::getLastId exceeded INT_MAX");

    return (unsigned) lastId;
}

bool SqLiteDataProvider::prepareSql(const std::string &sql)
{
    if (!mIsConnected)
        return false;

    LOG_DEBUG("Preparing SQL statement: "<<sql);

    mRecordSet.clear();

    if (sqlite3_prepare_v2(mDb, sql.c_str(), sql.size(),
            &mStmt, nullptr) != SQLITE_OK)
        return false;

    return true;
}

const RecordSet &SqLiteDataProvider::processSql()
{
    if (!mIsConnected)
        throw std::runtime_error("not connected to database");

    int totalCols = sqlite3_column_count(mStmt);

    // ensure we set column headers before adding a row
    Row fieldNames;
    for (int col = 0; col < totalCols; ++col)
    {
        fieldNames.push_back(sqlite3_column_name(mStmt, col));
    }
    mRecordSet.setColumnHeaders(fieldNames);

    while (sqlite3_step(mStmt) == SQLITE_ROW)
    {
        Row r;
        for (int col = 0; col < totalCols; ++col)
        {
            const unsigned char *txt = sqlite3_column_text(mStmt, col);
            r.push_back(txt ? (char*)txt : std::string());

        }
        mRecordSet.add(r);
    }

    sqlite3_finalize(mStmt);

    return mRecordSet;
}

void SqLiteDataProvider::bindValue(int place, const std::string &value)
{
    sqlite3_bind_text(mStmt, place, value.c_str(), value.size(), SQLITE_STATIC);
}

void SqLiteDataProvider::bindValue(int place, int value)
{
    sqlite3_bind_int(mStmt, place, value);
}

} // namespace dal
