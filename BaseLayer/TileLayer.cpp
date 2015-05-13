#include "stdafx.h"
#include "TileLayer.h"

TileLayer::TileLayer(int tileSize, int mapWidth, int mapHeight, bool breakable, bool collidable)
: m_tileSize(tileSize)
{
    m_numColumns = mapWidth;
    m_numRows = mapHeight;
    
    m_mapWidth = mapWidth;

	m_breakable = breakable;
	m_collidable = collidable;
}