#include "stdafx.h"
#include "TMXMap.h"


TMXMap::TMXMap(int tileSize, int width, int height)
: m_tileSize(tileSize)
, m_width(width)
, m_height(height)
{
}


TMXMap::~TMXMap()
{
}


void TMXMap::AddPlayerSpawnPoint(int x, int y)
{
	PlayerSpawnPoint point;
	point.x = x;
	point.y = y;

	m_playerSpawnPoint.push_back(point);
}

void TMXMap::AddTrigger(int x, int y, std::wstring triggerType)
{
	TriggerInfo info;
	info.x = x;
	info.y = y;
	info.type = 0;

	m_triggers.push_back(info);
}

void TMXMap::AddCollisionLayer(TileLayer* pLayer)
{
	m_collisionLayers.push_back(pLayer);
}

void TMXMap::AddTileLayer(TileLayer* pLayer)
{
	m_tileLayers.push_back(pLayer);
}