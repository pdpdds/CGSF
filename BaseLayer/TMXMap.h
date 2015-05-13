#pragma once
#include <vector>

class TileLayer;

typedef struct tag_PlayerSpawnPoint
{
	int x;
	int y;

	tag_PlayerSpawnPoint()
	{
		x = 0;
		y = 0;
	}
}PlayerSpawnPoint;

typedef struct tag_TriggerInfo
{
	int x;
	int y;
	int type;

	tag_TriggerInfo()
	{
		x = 0;
		y = 0;
		type = 0;
	}
}TriggerInfo;

class TMXMap
{
public:
	TMXMap(int tileSize, int width, int height);
	virtual ~TMXMap();

	void AddPlayerSpawnPoint(int x, int y);
	void AddTrigger(int x, int y, std::wstring triggerType);
	void AddCollisionLayer(TileLayer* pLayer);
	void AddTileLayer(TileLayer* pLayer);

private:
	int m_tileSize;
	int m_width;
	int m_height;

	std::vector<PlayerSpawnPoint> m_playerSpawnPoint;
	std::vector<TriggerInfo> m_triggers;
	std::vector<TileLayer*> m_tileLayers;
	std::vector<TileLayer*> m_collisionLayers;
};
