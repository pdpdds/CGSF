#pragma once

#include <iostream>
#include <vector>
#include "tinyxml.h"

struct Tileset;
class TMXMap;

class TMXParser
{
public: 
	TMXMap* parseTMX(TCHAR *tmxFile);
    
private:
	void parseObjectLayer(TiXmlElement* pObjectElement, TMXMap* pTMXMap);
	void parseTileLayer(TiXmlElement* pTileElement, TMXMap* pTMXMap);
    
    int m_tileSize;
    int m_width;
    int m_height;
};