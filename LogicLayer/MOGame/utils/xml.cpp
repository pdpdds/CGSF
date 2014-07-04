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

#include "utils/xml.h"

#include "common/resourcemanager.h"
#include "utils/logger.h"
#include "utils/string.h"

#include <iostream>
#include <fstream>

namespace XML
{
    Document::Document(const std::string &fileName, bool useResman):
        mDoc(0)
    {
        std::string resolvedFileName = fileName;
        if (useResman)
        {
            resolvedFileName = ResourceManager::resolve(fileName);

            if (resolvedFileName.empty())
            {
                LOG_ERROR("(XML::Document) File not found in search path: "
                          << fileName);
                return;
            }
        }

        mDoc = xmlParseFile(resolvedFileName.c_str());

        if (!mDoc)
        {
            LOG_ERROR("(XML::Document) Error parsing XML file: "
                      << resolvedFileName);
        }
    }

    Document::~Document()
    {
        if (mDoc)
            xmlFreeDoc(mDoc);
    }

    xmlNodePtr Document::rootNode()
    {
        return mDoc ? xmlDocGetRootElement(mDoc) : 0;
    }

    bool hasProperty(xmlNodePtr node, const char *name)
    {
        xmlChar *prop = xmlGetProp(node, BAD_CAST name);
        if (prop)
        {
            xmlFree(prop);
            return true;
        }

        return false;
    }

    bool getBoolProperty(xmlNodePtr node, const char *name, bool def)
    {
        bool ret = def;
        xmlChar *prop = xmlGetProp(node, BAD_CAST name);
        if (prop)
        {
            ret = utils::stringToBool((char*) prop, def);
            xmlFree(prop);
        }
        return ret;
    }

    int getProperty(xmlNodePtr node, const char *name, int def)
    {
        int &ret = def;

        xmlChar *prop = xmlGetProp(node, BAD_CAST name);
        if (prop)
        {
            ret = atol((char*) prop);
            xmlFree(prop);
        }

        return ret;
    }

    double getFloatProperty(xmlNodePtr node, const char *name, double def)
    {
        double &ret = def;

        xmlChar *prop = xmlGetProp(node, BAD_CAST name);
        if (prop)
        {
            ret = atof((char*) prop);
            xmlFree(prop);
        }

        return ret;
    }

    std::string getProperty(xmlNodePtr node, const char *name,
                            const std::string &def)
    {
        xmlChar *prop = xmlGetProp(node, BAD_CAST name);
        if (prop)
        {
            std::string val = (char*) prop;
            xmlFree(prop);
            return val;
        }

        return def;
    }

    xmlNodePtr findFirstChildByName(xmlNodePtr parent, const char *name)
    {
        for_each_xml_child_node(child, parent)
            if (xmlStrEqual(child->name, BAD_CAST name))
                return child;

        return nullptr;
    }

} // namespace XML
