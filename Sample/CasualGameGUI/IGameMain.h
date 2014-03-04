#pragma once

class BasePacket;
struct NetworkMessage;

class IGameMain
{
public:
	virtual bool Initialize() = 0;
    virtual bool Finally() = 0;
	virtual bool Notify(BasePacket* pPacket) = 0;
	virtual void Notify(NetworkMessage* pMsg) {}
	virtual bool ProcessInput(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
	virtual void OnRender(float fElapsedTime) = 0;
};

