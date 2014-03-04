#include "StdAfx.h"
#include "SGStateGame.h"
#include "SGManager.h"
#include "DirectXSystem.h"
#include "SevenGamePacketID.h"
#include "SevenGamePacket.pb.h"
#include "SFProtobufPacket.h"
#include "SFNetworkEntry.h"

extern SFNetworkEntry* g_pNetworkEntry;

SGStateGame::SGStateGame( SGManager* pManager )
: SGState(pManager)
{
}

SGStateGame::~SGStateGame(void)
{
}

BOOL SGStateGame::OnEnter()
{
	GetSGManager()->SetCardEffectTime(0.0f);
	return TRUE;
}

BOOL SGStateGame::OnMessage(int iX, int iY)
{
	BOOL bResult = FALSE;
	SGManager* pManager = GetSGManager();

	if(pManager->m_iCurrentTurn !=  pManager->GetMyID())
		return FALSE;

	bResult = GetSGManager()->CheckPassTicket(iX,iY);

	if(bResult == TRUE)
	{
		SFProtobufPacket<SevenGamePacket::TurnPass> turnPass(SevenGame::TurnPass);
	
		turnPass.GetData().set_playerindex(pManager->GetMyID());
		return g_pNetworkEntry->TCPSend(&turnPass);
	}

	SCardInfo cardInfo;
	bResult = pManager->UserProcessGame(iX, iY, cardInfo);

	if(bResult == TRUE)
	{
		SFProtobufPacket<SevenGamePacket::CardSubmit> cardSubmit(SevenGame::CardSubmit);
		cardSubmit.GetData().set_playerindex(pManager->GetMyID());
		cardSubmit.GetData().set_cardnum(cardInfo.iCardNum);
		cardSubmit.GetData().set_cardtype(cardInfo.iCardType);
		return g_pNetworkEntry->TCPSend(&cardSubmit);
	}



	return bResult;
}

BOOL SGStateGame::OnRender( float fElapsedTime )
{
	CDirectXSystem* pRenderer = CDirectXSystem::GetInstance();
	SGManager* pManager = GetSGManager();

	if(pRenderer)
	{
		pRenderer->DrawCard(fElapsedTime);	
		pRenderer->DrawPassButton(fElapsedTime);

		if(pManager->m_iCurrentTurn == pManager->GetMyID())		
			pRenderer->DrawCardTurn(fElapsedTime);
	}

	return TRUE;
}