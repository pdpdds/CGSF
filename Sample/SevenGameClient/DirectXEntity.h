#pragma once

class CDirectXEntity
{
public:
	CDirectXEntity(void);
	virtual ~CDirectXEntity(void);

	virtual BOOL OnRender(float fElapsedTime) = 0;
	virtual BOOL OnResetDevice() = 0;
	virtual BOOL OnFrameMove( double fTime, float fElapsedTime) = 0;
	virtual BOOL OnLostDevice() = 0;
	virtual BOOL OnDestroyDevice() = 0;

protected:

private:
};
