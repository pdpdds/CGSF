#pragma once
#include "SGState.h"

class SGManager;

class SGStateManager : public SGState
{
	typedef std::map<eSGState, SGState*> mapSGState;
public:
	SGStateManager(SGManager* pManager);
	virtual ~SGStateManager(void);

	BOOL AddState(eSGState state);
	BOOL ChangeState(eSGState state);

	virtual BOOL OnMessage(int iX, int iY) override;
	virtual BOOL OnRender(float fElapsedTime) override;

protected:

private:
	SGState* m_pCurrentSGState;
	mapSGState m_mapSGState;
	SGManager* m_pSGManager;
};
