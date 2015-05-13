#pragma once

#include <iostream>
#include <vector>

class TileLayer
{
public:
    
	TileLayer(int tileSize, int mapWidth, int mapHeight, bool breakable, bool collidable);
    
    virtual ~TileLayer() {}
    
    void setTileIDs(const std::vector<std::vector<int> >& data) { m_tileIDs = data; }
    void setTileSize(int tileSize) { m_tileSize = tileSize; }
    void setMapWidth(int mapWidth) { m_mapWidth = mapWidth; }
    int getMapWidth() { return m_mapWidth; }
	void setMapHeight(int mapHeight) { m_mapHeight = mapHeight; }
	int getMapHeight() { return m_mapHeight; }
    
    int getTileSize() { return m_tileSize; }
    
    const std::vector<std::vector<int> >& getTileIDs() { return m_tileIDs; }
    
	int getRows(){ return m_numRows; }
	int getCols(){ return m_numColumns; }  
    
private:
    
    int m_numColumns;
    int m_numRows;
    int m_tileSize;
    
    int m_mapWidth;
	int m_mapHeight;
    
	bool m_breakable;
	bool m_collidable;
    
protected:
    std::vector<std::vector<int> > m_tileIDs;
};
