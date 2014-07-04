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

#include "common/resourcemanager.h"

#include "common/configuration.h"

#include "utils/logger.h"

#include <sys/stat.h>
#include <cstdlib>
#include <cstring>
#include <vector>

#ifdef _WIN32
#include <io.h>
#include <direct.h>
#else
#include <unistd.h>
#include <dirent.h>
#endif

#include <physfs.h>

#ifndef PKG_DATADIR
#define PKG_DATADIR "."
#endif

void ResourceManager::initialize()
{
    PHYSFS_permitSymbolicLinks(1);

    const std::string worldDataPath =
            Configuration::getValue("worldDataPath", "example");

    // world first to allow overriding of server's libraries
    PHYSFS_addToSearchPath(worldDataPath.c_str(), 1);
    PHYSFS_addToSearchPath(".", 1);
    PHYSFS_addToSearchPath(PKG_DATADIR, 1);
}

/**
 * This function tries to check if a file exists based on the existence of
 * stats about it. Because simply trying to open it and check for failure is a
 * bad thing, as you don't know if you weren't able to open it because it
 * doesn't exist or because you don't have the right to.
 */
static bool fileExists(const std::string &filename)
{
    struct stat buffer;
    // When stat is succesful, the file exists
    return stat(filename.c_str(), &buffer) == 0;
}

bool ResourceManager::exists(const std::string &path, bool lookInSearchPath)
{
    if (!lookInSearchPath)
        return fileExists(path);

    return PHYSFS_exists(path.c_str());
}

std::string ResourceManager::resolve(const std::string &path)
{
    const char *realDir = PHYSFS_getRealDir(path.c_str());
    if (realDir)
        return std::string(realDir) + "/" + path;

    return std::string();
}

char *ResourceManager::loadFile(const std::string &fileName, int &fileSize)
{
    // Attempt to open the specified file using PhysicsFS
    PHYSFS_file *file = PHYSFS_openRead(fileName.c_str());

    // If the handler is an invalid pointer indicate failure
    if (file == nullptr)
    {
        LOG_WARN("Failed to load '" << fileName << "': "
                 << PHYSFS_getLastError());
        return nullptr;
    }

    // Get the size of the file
    fileSize = PHYSFS_fileLength(file);

    // Allocate memory and load the file
    char *buffer = (char *) malloc(fileSize + 1);
    if (PHYSFS_read(file, buffer, 1, fileSize) != fileSize)
    {
        free(buffer);
        LOG_WARN("Failed to load '" << fileName << "': "
                 << PHYSFS_getLastError());
        return nullptr;
    }

    // Close the file and let the user deallocate the memory
    PHYSFS_close(file);

    // Add a trailing null character, so that the file can be used as a string
    buffer[fileSize] = 0;
    return buffer;
}

ResourceManager::splittedPath ResourceManager::splitFileNameAndPath(
                                                const std::string &fullFilePath)
{
    // We'll reversed-search for '/' or'\' and extract the substrings
    // corresponding to the filename and the path separately.
    size_t slashPos = fullFilePath.find_last_of("/\\");

    ResourceManager::splittedPath splittedFilePath;
    // Note the last slash is kept in the path name.
    splittedFilePath.path = fullFilePath.substr(0, slashPos + 1);
    splittedFilePath.file = fullFilePath.substr(slashPos + 1);

    return splittedFilePath;
}

/**
 * Join two path elements into one.
 *
 * This function helps build relative paths.
 *
 * Examples:
 *
 *     /foo + bar = /foo/bar
 *     /foo/ + bar = /foo/bar
 *     /foo + /bar = /bar
 *
 * This will work for PhysFS paths. Windows style paths (prefixed with drive letters) won't work.
 *
 * @return Joined paths or path2 if path2 was an absolute path.
 */
std::string ResourceManager::joinPaths(const std::string& path1, const std::string& path2)
{
    if (path2.empty())
        return path1;

    if (path1.empty())
        return path2;

    // check if path2 is an absolute path that cannot be joined
    if (path2[0] == '/' || path2[0] == '\\')
        return path2;

    char p1end = path1[path1.size()-1];
    if (p1end == '/' || p1end == '\\')
    {
        return path1 + path2;
    }
    else
    {
        return path1 + "/" + path2;
    }
}

/**
 * Removes relative elements from the path.
 */
std::string ResourceManager::cleanPath(const std::string& path)
{
    size_t prev, cur;
    std::string part, result;
    std::vector<std::string> pathStack;

    prev = 0;
    while (true)
    {
        cur = path.find_first_of("/\\", prev);
        if (cur == std::string::npos)
        {
            // FIXME add everything from prev to the end
            pathStack.push_back(path.substr(prev));
            break;
        }

        part = path.substr(prev, cur - prev);
        if (part == "..")
        {
            // go back one level
            if (!pathStack.empty())
            {
                pathStack.pop_back();
            }
        }
        else if (part == ".")
        {
            // do nothing
        }
        else if (part == "")
        {
            if (pathStack.empty() && cur == 0)
            {
                // handle first empty match before the root slash
                pathStack.push_back(std::string());
            }
            else
            {
                // empty match in the middle of the path should be ignored
            }
        }
        else
        {
            // normal path element
            pathStack.push_back(part);
        }

        cur++;
        prev = cur;
    }

    // join the pathStack into a normal path
    unsigned int i = 0;
    for (i = 0; i < pathStack.size(); i++)
    {
        result += pathStack[i];
        if (i < pathStack.size() - 1) {
            result += "/";
        }
    }

    return result;
}
