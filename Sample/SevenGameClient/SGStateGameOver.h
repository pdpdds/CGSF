#pragma once
#include "SGState.h"

class SGStateGameOver : public SGState
{
public:
	SGStateGameOver( SGManager* pManager );
	virtual ~SGStateGameOver(void);

	virtual BOOL OnEnter() override;
	virtual BOOL OnMessage(int iX, int iY) override;
	virtual BOOL OnRender(float fElapsedTime) override;

protected:

private:

};
