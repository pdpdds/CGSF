/*
 *  The Mana Server
 *  Copyright (C) 2013  The Mana World Development Team
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

#ifndef GAMESERVER_SETTINGSMANAGER_H_
#define GAMESERVER_SETTINGSMANAGER_H_

#include <string>
#include <list>
#include <set>

class SettingsManager
{
    public:
        SettingsManager(const std::string &settingsFile):
			mSettingsFile(settingsFile)
		{}

		void initialize();

		void reload();

	private:
		std::string mSettingsFile;
		std::set<std::string> mIncludedFiles;

		void loadFile(const std::string &filename);

		void checkStatus();
};


extern SettingsManager *settingsManager;

#endif // GAMESERVER_SETTINGSMANAGER_H_
