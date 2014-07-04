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

#ifndef RECORDSET_H
#define RECORDSET_H

#include <iostream>
#include <vector>

namespace dal
{

/**
 * Data type for a row in a RecordSet.
 */
typedef std::vector<std::string> Row;


/**
 * A RecordSet to store the result of a SQL query.
 *
 * Limitations:
 *     - the field values are stored and returned as string,
 *     - no information about the field data types are stored.
 *     - not thread-safe.
 */
class RecordSet
{
    public:
        RecordSet()
            throw();

        ~RecordSet()
            throw();

        /**
         * Remove all the records.
         */
        void clear();

        /**
         * Check if the RecordSet is empty.
         *
         * @return true if empty.
         */
        bool isEmpty() const;

        /**
         * Get the number of rows.
         *
         * @return the number of rows.
         */
        unsigned rows() const;

        /**
         * Get the number of columns.
         *
         * @return the number of columns.
         */
        unsigned cols() const;

        /**
         * Set the column headers.
         *
         * @param headers the column headers.
         *
         * @exception AlreadySetException if the column headers
         *            are already set.
         */
        void setColumnHeaders(const Row &headers);

        /**
         * Add a new row.
         *
         * This method does not check the field data types, only the number
         * of columns is checked.
         *
         * @param row the new row.
         *
         * @exception RsColumnHeadersNotSet if the row is being added before
         *            the column headers.
         * @exception std::invalid_argument if the number of columns in the
         *            new row is not equal to the number of column headers.
         */
        void add(const Row &row);

        /**
         * Operator()
         * Get the value of a particular field of a particular row
         * by field index.
         *
         * @param row the row index.
         * @param col the field index.
         *
         * @return the field value.
         *
         * @exception std::out_of_range if row or col are out of range.
         * @exception std::invalid_argument if the recordset is empty.
         */
        const std::string&
        operator()(const unsigned row,
                   const unsigned col) const;


        /**
         * Operator()
         * Get the value of a particular field of a particular row
         * by field name (slower than by field index).
         *
         * @param row the row index.
         * @param name the field name.
         *
         * @return the field value.
         *
         * @exception std::out_of_range if the row index is out of range.
         * @exception std::invalid_argument if the field name is not found or
         *            the recordset is empty.
         */
        const std::string&
        operator()(const unsigned row,
                   const std::string &name) const;


        /**
         * Operator<<
         * Append the stringified RecordSet to the output stream.
         *
         * @param out the output stream.
         * @param rhs the right hand side.
         *
         * @return the output stream for chaining.
         */
        friend std::ostream&
        operator<<(std::ostream& out, const RecordSet &rhs);

    private:
        /**
         * Copy constructor.
         */
        RecordSet(const RecordSet &rhs);

        /**
         * Assignment operator.
         */
        RecordSet&
        operator=(const RecordSet &rhs);


    private:
        Row mHeaders; /**< a list of field names */
        typedef std::vector<Row> Rows;
        Rows mRows;   /**< a list of records */
};


} // namespace dal

#endif // RECORDSET_H
