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

#include <algorithm>
#include <sstream>
#include <stdexcept>

#include "dal/recordset.h"

#include "dal/dalexcept.h"

namespace dal
{

RecordSet::RecordSet()
    throw()
{
}

RecordSet::~RecordSet()
    throw()
{
}

/**
 * Remove all the Records.
 */
void RecordSet::clear()
{
    mHeaders.clear();
    mRows.clear();
}

/**
 * Check if the RecordSet is empty.
 */
bool RecordSet::isEmpty() const
{
    return mRows.size() == 0;
}

/**
 * Get the number of rows.
 *
 * @return the number of rows.
 */
unsigned RecordSet::rows() const
{
    return mRows.size();
}

/**
 * Get the number of columns.
 *
 * @return the number of columns.
 */
unsigned RecordSet::cols() const
{
    return mHeaders.size();
}

/**
 * Set the column headers.
 */
void RecordSet::setColumnHeaders(const Row &headers)
{
    if (mHeaders.size() > 0) {
        throw AlreadySetException();
    }

    mHeaders = headers;
}

/**
 * Add a new row.
 */
void RecordSet::add(const Row &row)
{
    const unsigned nCols = mHeaders.size();

    if (nCols == 0) {
        throw RsColumnHeadersNotSet();
    }

    if (row.size() != nCols) {
        std::ostringstream msg;
        msg << "row has " << row.size() << " columns; "
            << "expected: " << nCols << std::ends;

        throw std::invalid_argument(msg.str());
    }

    mRows.push_back(row);
}

const std::string &RecordSet::operator()(const unsigned row,
                                         const unsigned col) const
{
    if ((row >= mRows.size()) || (col >= mHeaders.size())) {
        std::ostringstream os;
        os << "(" << row << ", " << col << ") is out of range; "
           << "max rows: " << mRows.size()
           << ", max cols: " << mHeaders.size() << std::ends;

        throw std::out_of_range(os.str());
    }

    return mRows[row][col];
}

const std::string &RecordSet::operator()(const unsigned row,
                                         const std::string& name) const
{
    if (row >= mRows.size()) {
        std::ostringstream os;
        os << "row " << row << " is out of range; "
           << "max rows: " << mRows.size() << std::ends;

        throw std::out_of_range(os.str());
    }

    Row::const_iterator it = std::find(mHeaders.begin(),
                                       mHeaders.end(),
                                       name);
    if (it == mHeaders.end()) {
        std::ostringstream os;
        os << "field " << name << " does not exist." << std::ends;

        throw std::invalid_argument(os.str());
    }

    // find the field index.
    const unsigned nCols = mHeaders.size();
    unsigned i;
    for (i = 0; i < nCols; ++i) {
        if (mHeaders[i] == name) {
            break;
        }
    }

    return mRows[row][i];
}

std::ostream &operator<<(std::ostream &out, const RecordSet &rhs)
{
    // print the field names first.
    if (rhs.mHeaders.size() > 0) {
        out << "|";
        for (Row::const_iterator it = rhs.mHeaders.begin();
             it != rhs.mHeaders.end();
             ++it)
        {
            out << (*it) << "|";
        }
        out << std::endl << std::endl;
    }

    // and then print every line.
    for (RecordSet::Rows::const_iterator it = rhs.mRows.begin();
         it != rhs.mRows.end();
         ++it)
    {
        out << "|";
        for (Row::const_iterator it2 = (*it).begin();
             it2 != (*it).end();
             ++it2)
        {
            out << (*it2) << "|";
        }
        out << std::endl;
    }

    return out;
}

} // namespace dal
