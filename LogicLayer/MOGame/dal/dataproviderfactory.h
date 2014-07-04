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

#ifndef DATA_PROVIDER_FACTORY_H
#define DATA_PROVIDER_FACTORY_H

namespace dal
{
    class DataProvider;


/**
 * A factory to create data providers.
 *
 * Note:
 *     - this class does not assume the ownership of the pointers returned
 *       by createDataProvider().
 *     - this class is a singleton.
 */
class DataProviderFactory
{
    public:
        /**
         * Create a new data provider.
         */
        static DataProvider *createDataProvider();

    private:
        /**
         * Default constructor.
         */
        DataProviderFactory()
            throw();

        ~DataProviderFactory()
            throw();

        /**
         * Copy constructor.
         */
        DataProviderFactory(const DataProviderFactory &rhs);


        /**
         * Assignment operator.
         */
        DataProviderFactory&
        operator=(const DataProviderFactory &rhs);
};


} // namespace dal

#endif // DATA_PROVIDER_FACTORY_H
