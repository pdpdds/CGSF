
#ifndef WAIT_H
#define WAIT_H

//-----------------------------------------------------------------------------
// State ID Define
//-----------------------------------------------------------------------------
#define STATE_WAIT 0

//-----------------------------------------------------------------------------
// Menu Class
//-----------------------------------------------------------------------------
class Wait : public State
{
public:
	Wait();

	virtual void Update( float elapsed );
	virtual void HandleNetworkMessage(BasePacket* pPacket) override;
	virtual void HandleNetworkMessage( NetworkMessage* pMessage ) override;

	virtual void Render(float elapsed) override;
	virtual bool HandleInput(UINT msg, WPARAM wparam, LPARAM lparam) override;

};

#endif