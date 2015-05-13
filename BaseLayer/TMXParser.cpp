#include "stdafx.h"
#include <string>
#include "TMXParser.h"
#include "base64.h"
#include "zlib/zlib.h"
#include "TMXMap.h"
#include "TileLayer.h"

TMXMap* TMXParser::parseTMX(TCHAR *tmxFile)
{
	TiXmlDocument levelDocument;

	levelDocument.LoadFile(tmxFile);
    
    // get the root node and display some values
    TiXmlElement* pRoot = levelDocument.RootElement();
    
    std::cout << "Loading level:\n" << "Version: " << pRoot->Attribute(L"version") << "\n";
    std::cout << "Width:" << pRoot->Attribute(L"width") << " - Height:" << pRoot->Attribute(L"height") << "\n";
    std::cout << "Tile Width:" << pRoot->Attribute(L"tilewidth") << " - Tile Height:" << pRoot->Attribute(L"tileheight") << "\n";
    
    pRoot->Attribute(L"tilewidth", &m_tileSize);
    pRoot->Attribute(L"width", &m_width);
    pRoot->Attribute(L"height", &m_height);

	TMXMap* pTMXMap = new TMXMap(m_tileSize, m_width, m_height);

    //we know that properties is the first child of the root
    TiXmlElement* pProperties = pRoot->FirstChildElement();
    
    for(TiXmlElement* e = pProperties->FirstChildElement(); e != NULL; e = e->NextSiblingElement())
    {
        if(e->Value() == std::wstring(L"property"))
        {
           
        }
    }
    
    for(TiXmlElement* e = pRoot->FirstChildElement(); e != NULL; e = e->NextSiblingElement())
    {
		if (e->Value() == std::wstring(L"tileset"))
        {
            
        }
    }
    
 //오브젝트 레이어, 타일 레이어 파싱
    for(TiXmlElement* e = pRoot->FirstChildElement(); e != NULL; e = e->NextSiblingElement())
    {
		if (e->Value() == std::wstring(L"objectgroup") || e->Value() == std::wstring(L"layer"))
        {
			std::wstring aa = e->FirstChildElement()->Value();
			if (e->FirstChildElement()->Value() == std::wstring(L"object"))
            {
				parseObjectLayer(e, pTMXMap);
            }
            else if(e->FirstChildElement()->Value() == std::wstring(L"data") ||
                    (e->FirstChildElement()->NextSiblingElement() != 0 && e->FirstChildElement()->NextSiblingElement()->Value() == std::wstring(L"data")))
            {
                parseTileLayer(e, pTMXMap);
            }
			else if 
				(e->FirstChildElement()->NextSiblingElement() != 0 && e->FirstChildElement()->NextSiblingElement()->Value() == std::wstring(L"object"))
			{
				parseObjectLayer(e, pTMXMap);
			}
        }
    }
	
	return pTMXMap;
}

void TMXParser::parseObjectLayer(TiXmlElement* pObjectElement, TMXMap* pTMXMap)
{
    // create an object layer
    //ObjectLayer* pObjectLayer = new ObjectLayer();
    
    for(TiXmlElement* e = pObjectElement->FirstChildElement(); e != NULL; e = e->NextSiblingElement())
    {
        if(e->Value() == std::wstring(L"object"))
        {
            int x, y, width, height, numFrames, callbackID = 0, animSpeed = 0;
            std::wstring textureID;
			std::wstring triggerType;
            std::wstring type;
            
            // get the initial node values type, x and y
            e->Attribute(L"x", &x);
            e->Attribute(L"y", &y);
            
            type = e->Attribute(L"type");
            
			if (type == L"Player")
			{

				// get the property values
				for (TiXmlElement* properties = e->FirstChildElement(); properties != NULL; properties = properties->NextSiblingElement())
				{
					if (properties->Value() == std::wstring(L"properties"))
					{
						for (TiXmlElement* property = properties->FirstChildElement(); property != NULL; property = property->NextSiblingElement())
						{
							if (property->Value() == std::wstring(L"property"))
							{
								if (property->Attribute(L"name") == std::wstring(L"numFrames"))
								{
									property->Attribute(L"value", &numFrames);
								}
								else if (property->Attribute(L"name") == std::wstring(L"textureHeight"))
								{
									property->Attribute(L"value", &height);
								}
								else if (property->Attribute(L"name") == std::wstring(L"textureID"))
								{
									textureID = property->Attribute(L"value");
								}
								else if (property->Attribute(L"name") == std::wstring(L"textureWidth"))
								{
									property->Attribute(L"value", &width);
								}
								else if (property->Attribute(L"name") == std::wstring(L"callbackID"))
								{
									property->Attribute(L"value", &callbackID);
								}
								else if (e->Attribute(L"name") == std::wstring(L"animSpeed"))
								{
									property->Attribute(L"value", &animSpeed);
								}
							}
						}
					}
				}

				pTMXMap->AddPlayerSpawnPoint(x, y);
			}
			else if (type == L"Trigger")
			{
				// get the property values
				for (TiXmlElement* properties = e->FirstChildElement(); properties != NULL; properties = properties->NextSiblingElement())
				{
					if (properties->Value() == std::wstring(L"properties"))
					{
						for (TiXmlElement* property = properties->FirstChildElement(); property != NULL; property = property->NextSiblingElement())
						{
							if (property->Value() == std::wstring(L"property"))
							{
								if (property->Attribute(L"name") == std::wstring(L"triggerType"))
								{
									triggerType = property->Attribute(L"value");

									pTMXMap->AddTrigger(x, y, triggerType);
								}								
							}
						}
					}
				}

				
			}
        }
    }
}

void TMXParser::parseTileLayer(TiXmlElement* pTileElement, TMXMap* pTMXMap)
{
	TileLayer* pTileLayer = 0;
	
    bool collidable = false;
	bool breakable = false;
    
    // tile data
    std::vector<std::vector<int>> data;
    
    std::string decodedIDs;
    TiXmlElement* pDataNode = 0;
    
    for(TiXmlElement* e = pTileElement->FirstChildElement(); e != NULL; e = e->NextSiblingElement())
    {
		if (e->Value() == std::wstring(L"properties"))
        {
            for(TiXmlElement* property = e->FirstChildElement(); property != NULL; property = property->NextSiblingElement())
            {
				if (property->Value() == std::wstring(L"property"))
                {
					if (property->Attribute(L"name") == std::wstring(L"collidable"))
                    {
                        collidable = true;
                    }
					if (property->Attribute(L"name") == std::wstring(L"breakable"))
					{
						breakable = true;
					}
                }
            }
        }
        
		if (e->Value() == std::wstring(L"data"))
        {
            pDataNode = e;
        }
    }
    
    for(TiXmlNode* e = pDataNode->FirstChild(); e != NULL; e = e->NextSibling())
    {
        TiXmlText* text = e->ToText();
		std::wstring t = text->Value();
		std::string str;
		str.assign(t.begin(), t.end());
		decodedIDs = base64_decode(str);
    }
    
    // uncompress zlib compression
    uLongf sizeofids = m_width * m_height * sizeof(int);
    std::vector<int> ids(m_width * m_height);
    uncompress((Bytef*)&ids[0], &sizeofids,(const Bytef*)decodedIDs.c_str(), decodedIDs.size());
    	
	pTileLayer = new TileLayer(m_tileSize, m_width, m_height, breakable, collidable);
	    
	std::vector<int> layerRow(m_width);
    
    for(int j = 0; j < m_height; j++)
    {
        data.push_back(layerRow);
    }
    
    for(int rows = 0; rows < m_height; rows++)
    {
        for(int cols = 0; cols < m_width; cols++)
        {
            data[rows][cols] = ids[rows * m_width + cols];
		}
	}

	pTileLayer->setTileIDs(data);

	if (collidable)
	{
		pTMXMap->AddCollisionLayer(pTileLayer);
	}

	pTMXMap->AddTileLayer(pTileLayer);
}