/*
 *  XML utility functions
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
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


#ifndef XML_H
#define XML_H

#include "libxml/tree.h"

#include <string>

/**
 * XML helper functions.
 */
namespace XML
{
    /**
     * A helper class for parsing an XML document, which also cleans it up
     * again (RAII).
     */
    class Document
    {
        public:
            /**
             * Attempts to load an XML document from the given file. Logs an
             * error when something went wrong.
             *
             * @param fileName  the file name of the XML document
             * @param useResman whether to resolve the full path to the file
             *                  using the resource manager (true by default).
             */
            Document(const std::string &fileName, bool useResman = true);

            /**
             * Destructor. Frees the loaded XML file.
             */
            ~Document();

            /**
             * Returns the root node of the document (or nullptr if there was a
             * load error).
             */
            xmlNodePtr rootNode();

        private:
            xmlDocPtr mDoc;
    };

    /**
     * Tells if a property from an xmlNodePtr exists.
     */
    bool hasProperty(xmlNodePtr node, const char *name);

    /**
     * Gets a boolean property from an xmlNodePtr.
     */
    bool getBoolProperty(xmlNodePtr node, const char *name, bool def);

    /**
     * Gets an integer property from an xmlNodePtr.
     */
    int getProperty(xmlNodePtr node, const char *name, int def);

    /**
     * Gets an floating point property from an xmlNodePtr.
     */
    double getFloatProperty(xmlNodePtr node, const char *name, double def);

    /**
     * Gets a string property from an xmlNodePtr.
     */
    std::string getProperty(xmlNodePtr node, const char *name,
                            const std::string &def);

    /**
     * Finds the first child node with the given name
     */
    xmlNodePtr findFirstChildByName(xmlNodePtr parent, const char *name);
}

#define for_each_xml_child_node(var, parent) \
    for (xmlNodePtr var = parent->xmlChildrenNode; var; var = var->next)

#endif // XML_H
