#pragma once
#include "SGState.h"

class SGStateCardEffect : public SGState
{
public:
	SGStateCardEffect( SGManager* pManager );
	virtual ~SGStateCardEffect(void);

	virtual BOOL OnEnter() override;
	
	virtual BOOL OnRender(float fElapsedTime) override;
};
