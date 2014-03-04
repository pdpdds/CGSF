#pragma once
#include "SGState.h"

class SGUser;

class SGStateCardPassEffect : public SGState
{
public:
	SGStateCardPassEffect(SGManager* pManager);
	virtual ~SGStateCardPassEffect(void);

	virtual BOOL OnEnter() override;

	virtual BOOL OnRender(float fElapsedTime) override;

protected:

private:
	SGUser* m_pCurrentUser;
};
