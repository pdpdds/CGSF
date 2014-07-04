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
#include <string>
#include "utils/stringfilter.h"

#include "common/configuration.h"
#include "utils/logger.h"

namespace utils
{

StringFilter::StringFilter():
    mInitialized(false)
{
    loadSlangFilterList();
}

StringFilter::~StringFilter()
{
    writeSlangFilterList();
}

bool StringFilter::loadSlangFilterList()
{
    mInitialized = false;

    const std::string slangsList = Configuration::getValue("SlangsList",
                                                           std::string());
    if (!slangsList.empty()) {
        std::istringstream iss(slangsList);
        std::string tmp;
        while (getline(iss, tmp, ','))
            mSlangs.push_back(tmp);
        mInitialized = true;
    }

    return mInitialized;
}

void StringFilter::writeSlangFilterList()
{
    // Write the list to config
    std::string slangsList;
    for (SlangIterator i = mSlangs.begin(); i != mSlangs.end(); )
    {
        slangsList += *i;
        ++i;
        if (i != mSlangs.end()) slangsList += ",";
    }
    //mConfig->setValue("SlangsList", slangsList);
}

bool StringFilter::filterContent(const std::string &text) const
{
    if (!mInitialized) {
        LOG_DEBUG("Slangs List is not initialized.");
        return true;
    }

    bool isContentClean = true;
    std::string upperCaseText = text;

    std::transform(text.begin(), text.end(), upperCaseText.begin(),
            (int(*)(int))std::uppercase);

    for (Slangs::const_iterator i = mSlangs.begin(); i != mSlangs.end(); ++i)
    {
        // We look for slangs into the sentence.
        std::string upperCaseSlang = *i;
        std::transform(upperCaseSlang.begin(), upperCaseSlang.end(),
                upperCaseSlang.begin(), (int(*)(int))std::uppercase);

        if (upperCaseText.compare(upperCaseSlang)) {
            isContentClean = false;
            break;
        }
    }

    return isContentClean;
}

bool StringFilter::isEmailValid(const std::string &email) const
{
    unsigned min = Configuration::getValue("account_minEmailLength", 7);
    unsigned max = Configuration::getValue("account_maxEmailLength", 128);

    // Testing email validity
    if (email.length() < min || email.length() > max)
    {
        return false;
    }

    std::string::size_type atpos = email.find_first_of('@');

    // TODO Find some nice regex for this...
    return (atpos != std::string::npos) &&
        (email.find_first_of('.', atpos) != std::string::npos) &&
        (email.find_first_of(' ') == std::string::npos);
}

bool StringFilter::findDoubleQuotes(const std::string &text) const
{
    return (text.find('"', 0) != std::string::npos);
}

} // ::utils
