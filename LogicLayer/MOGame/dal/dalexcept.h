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

#ifndef DAL_EXCEPT_H
#define DAL_EXCEPT_H


#include <string>

namespace dal
{

/**
 * Default database exception.
 */
class DbException: public std::exception
{
    public:
        /**
         * Constructor.
         *
         * @param msg the error message.
         */
        DbException(const std::string &msg)
            throw()
                : mMsg(msg)
        {}

        ~DbException()
            throw()
        {}

        /**
         * Get the error message.
         *
         * @return the error message.
         */
        virtual const char *what() const
            throw()
        {
            return mMsg.c_str();
        }


    private:
        std::string mMsg;
};


/**
 * Database connection failure.
 */
class DbConnectionFailure: public DbException
{
    public:
        /**
         * Default constructor.
         */
        DbConnectionFailure()
            throw()
                : DbException(std::string())
        {}


        /**
         * Constructor.
         *
         * @param msg the error message.
         */
        DbConnectionFailure(const std::string& msg)
            throw()
                : DbException(msg)
        {}
};


/**
 * Database disconnection failure.
 */
class DbDisconnectionFailure: public DbException
{
    public:
        /**
         * Default constructor.
         */
        DbDisconnectionFailure()
            throw()
                : DbException(std::string())
        {}

        /**
         * Constructor.
         *
         * @param msg the error message.
         */
        DbDisconnectionFailure(const std::string& msg)
            throw()
                : DbException(msg)
        {}
};


/**
 * SQL query execution failure.
 */
class DbSqlQueryExecFailure: public DbException
{
    public:
        /**
         * Default constructor.
         */
        DbSqlQueryExecFailure()
            throw()
                : DbException(std::string())
        {}

        /**
         * Constructor.
         *
         * @param msg the error message.
         */
        DbSqlQueryExecFailure(const std::string& msg)
            throw()
                : DbException(msg)
        {}
};


/**
 * Already set exception.
 */
class AlreadySetException: public std::exception
{
};


/**
 * Missing column headers exception.
 */
class RsColumnHeadersNotSet: public std::exception
{
};


} // namespace dal

#endif // DAL_EXCEPT_H
