#pragma once
#include "SGState.h"

class SGStateGame : public SGState
{
public:
	SGStateGame( SGManager* pManager );
	~SGStateGame(void);

	virtual BOOL OnEnter() override;
	virtual BOOL OnMessage(int iX, int iY) override;
	virtual BOOL OnRender(float fElapsedTime) override;
};
