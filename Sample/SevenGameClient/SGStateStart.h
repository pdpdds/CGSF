#pragma once
#include "SGState.h"

class SGStateStart : public SGState
{
public:
	SGStateStart( SGManager* pManager );
	virtual ~SGStateStart(void);

	virtual BOOL OnEnter() override;
	virtual BOOL OnRender(float fElapsedTime) override;
};
