#pragma once

class SGManager;

class SGState
{
public:
	SGState(SGManager* pManager);
	virtual ~SGState(void);

	virtual BOOL OnEnter(){return FALSE;};
	virtual BOOL OnLeave(){return FALSE;};
	virtual BOOL OnMessage(int iX, int iY){return FALSE;};
	virtual BOOL OnRender(float fElpasedTime){return FALSE;};

protected:
	SGManager* GetSGManager(){return m_pManager;}

private:
	SGManager* m_pManager;
};