#pragma once
#include "IGameMain.h"

class FPSGameMain : public IGameMain
{
public:
	FPSGameMain(void);
	virtual ~FPSGameMain(void);

	virtual bool Initialize() override;
    virtual bool Finally() override;
	virtual bool Notify(BasePacket* pPacket) override;
	virtual void Notify(NetworkMessage* pMsg) override;
	bool ProcessInput(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	virtual void OnRender(float fElapsedTime) override;
};

