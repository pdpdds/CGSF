#pragma once

class CRenderer
{
public:
	CRenderer(void);
	virtual ~CRenderer(void);

	virtual BOOL OnRenderRegion(float fElapsedTime, 
		int	  iImageIndex,	
		float fPosX, 
		float fPosY, 
		float fColRatio, 
		float fRowRatio,
		float fColUnitRatio, 
		float fRowUnitRatio,
		float fRegionWidth, 
		float fRegionHeight) = 0;

	virtual BOOL OnRender(float fElapsedTime, 
		int	  iImageIndex,
		float fPosX, 
		float fPosY) = 0;
};
