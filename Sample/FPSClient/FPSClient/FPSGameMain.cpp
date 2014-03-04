#include "stdafx.h"
#include "FPSGameMain.h"
#include "FPSMain.h"


FPSGameMain::FPSGameMain(void)
{

}


FPSGameMain::~FPSGameMain(void)
{
}

bool FPSGameMain::Initialize()
{
	g_engine->ChangeState(STATE_GAME);
	return true;
}
    
bool FPSGameMain::Finally() 
{
	g_engine->ChangeState(STATE_WAIT);
	return true;

}

bool FPSGameMain::Notify(BasePacket* pPacket) 
{
	g_engine->GetCurrentState()->HandleNetworkMessage(pPacket);
	return true;
}

void FPSGameMain::Notify(NetworkMessage* pMsg)
{
	g_engine->GetCurrentState()->HandleNetworkMessage(pMsg);
	
}

bool FPSGameMain::ProcessInput(UINT uMsg, WPARAM wParam, LPARAM lParam)
{


	return true;
}

void FPSGameMain::OnRender(float fElapsedTime)
{
}