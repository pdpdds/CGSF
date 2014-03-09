#include "stdafx.h"
#include "SevenGameMain.h"
#include "SGManager.h"
#include "DirectXSystem.h"
#include "DXUT.h"
#include "BasePacket.h"
#include <SFPacketStore/SevenGamePacketID.h>
#include <SFPacketStore/SevenGamePacket.pb.h>
#include "SFProtobufPacket.h"
#include "SGUser.h"
#include "SGTable.h"

#pragma comment(lib, "CasualGame.lib")

SevenGameMain::SevenGameMain(void)
{
	m_bPassButtonOverapped = FALSE;
	m_SevenGameManger = NULL;
}


SevenGameMain::~SevenGameMain(void)
{
}

bool SevenGameMain::Initialize()
{
	if(m_SevenGameManger)
		delete m_SevenGameManger;

	m_SevenGameManger = new SGManager();
	m_SevenGameManger->AllocateObjcet(4,3);
	m_SevenGameManger->InitializeData();
	
	return true;
}
    
bool SevenGameMain::Finally() 
{
	delete m_SevenGameManger;
	m_SevenGameManger = 0;
	return true;
}

bool SevenGameMain::Notify(BasePacket* pPacket) 
{
	switch(pPacket->GetPacketID())
	{
	case SevenGame::InitCardCount:
		{
			InitializeTable(pPacket);
		}
		break;
	case SevenGame::PlayerID:
		{			
			SetPlayerID(pPacket);
		}
		break;

	case SevenGame::MyCardInfo:
		{			
			SetMyCard(pPacket);
		}
		break;

	case SevenGame::TableUpdate:
		{
			UpdateTable(pPacket);			
		}
		break;

	case SevenGame::CurrentTurn:
		{
			SetCurrentTurn(pPacket);			
		}
		break;

	case SevenGame::Winner:
		{
			OnWinner(pPacket);
		}
		break;

	case SevenGame::UserDie:
		{
			UserDie(pPacket);
			
		}
		break;

	case SevenGame::TurnPass:
		{
			TurnPass(pPacket);
		}
		break;

	case SevenGame::CardSubmit:
		{
			CardSubmit(pPacket);
		}
		break;
	}

	return true;
}

bool SevenGameMain::ProcessInput(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch( uMsg )
	{
	case WM_MOUSEMOVE:
		m_iMouseButtons = wParam;
		m_iMouseX       = LOWORD(lParam);
		m_iMouseY       = HIWORD(lParam) < CDirectXSystem::GetInstance()->GetHeight() ? HIWORD(lParam) : 0;		

		if(m_SevenGameManger->MouseInPassButton(m_iMouseX,m_iMouseY))
		{
			m_bPassButtonOverapped = TRUE;
		}
		else
			m_bPassButtonOverapped = FALSE;

		break;

	case WM_LBUTTONUP:
		{

			int iPosX = (int)LOWORD(lParam);     
			int iPosY = (int)HIWORD(lParam);

			m_SevenGameManger->OnMessage(iPosX, iPosY);

		}
		break;
	}

	return true;
}

void SevenGameMain::OnRender(float fElapsedTime)
{
	IDirect3DDevice9* pDevice = CDirectXSystem::GetInstance()->GetD3DDevice();

	m_SevenGameManger->OnRender(fElapsedTime);
}

void SevenGameMain::InitializeTable(BasePacket* pPacket)
{	
	m_SevenGameManger->InitializeData();

	SFProtobufPacket<SevenGamePacket::InitCardCount>* pInitCardCount = (SFProtobufPacket<SevenGamePacket::InitCardCount>*)pPacket;
	int playerCount = pInitCardCount->GetData().info_size();

	m_SevenGameManger->RemoveAllUser();
	m_SevenGameManger->m_vecActivePlayer.clear();

	for(int i = 0; i < playerCount; i++)
	{		
		const SevenGamePacket::InitCardCount::CardCount& info = pInitCardCount->GetData().info(i);

		int playerIndex = info.playerindex();
		m_SevenGameManger->m_vecActivePlayer.push_back(playerIndex);
		m_SevenGameManger->AddUser(playerIndex);

		SGUser* pUser = m_SevenGameManger->FindUser(playerIndex);
		pUser->Initialize();
		pUser->SetRemainCard(info.cardcount());
		pUser->SetPassTicket(3);
	}

	MakeDisplayOrder();
}

void SevenGameMain::MakeDisplayOrder()
{
	SGUser* pMe = m_SevenGameManger->FindUser(m_SevenGameManger->GetMyID());
	
	std::vector<int>::iterator iter = m_SevenGameManger->m_vecActivePlayer.begin();

	for(;iter != m_SevenGameManger->m_vecActivePlayer.end();iter++)
	{
		if(pMe->GetID() == *iter)
			break;
	}

	m_SevenGameManger->m_vecDisplayOrder.clear();

	while(iter != m_SevenGameManger->m_vecActivePlayer.end())
	{
		m_SevenGameManger->m_vecDisplayOrder.push_back(*iter);
		iter++;
	}

	iter = m_SevenGameManger->m_vecActivePlayer.begin();

	while(iter != m_SevenGameManger->m_vecActivePlayer.end())
	{
		if(pMe->GetID() == *iter)
			break;

		m_SevenGameManger->m_vecDisplayOrder.push_back(*iter);
		iter++;
	}
}


void SevenGameMain::SetPlayerID(BasePacket* pPacket)
{
	SFProtobufPacket<SevenGamePacket::PlayerID>* pPlayerID = (SFProtobufPacket<SevenGamePacket::PlayerID>*)pPacket;

	m_SevenGameManger->SetMyID(pPlayerID->GetData().playerindex());	
}


void SevenGameMain::SetMyCard(BasePacket* pPacket)
{
	SFProtobufPacket<SevenGamePacket::MyCardInfo>* pCardInfo = (SFProtobufPacket<SevenGamePacket::MyCardInfo>*)pPacket;
	int cardCount = pCardInfo->GetData().card_size();

	SGUser* pMe = m_SevenGameManger->FindUser(m_SevenGameManger->GetMyID());

	for(int i = 0; i < cardCount; i++)
	{		
		const SevenGamePacket::MyCardInfo::PlayerCard& info = pCardInfo->GetData().card(i);

		pMe->AddCard(info.cardtype(), info.cardnum());
	}
	
	m_SevenGameManger->ChangeState(ENUM_SGSTART);
	
}


void SevenGameMain::UpdateTable(BasePacket* pPacket)
{
	SFProtobufPacket<SevenGamePacket::TableUpdate>* pTableUpdate = (SFProtobufPacket<SevenGamePacket::TableUpdate>*)pPacket;
	SGTable* pTable = m_SevenGameManger->GetTable();

	int cardCount = pTableUpdate->GetData().card_size();

	for(int i = 0; i < cardCount; i++)
	{		
		const SevenGamePacket::TableUpdate::AddCard& info = pTableUpdate->GetData().card(i);

		int cardNum = info.cardnum();
		int cardType = info.cardtype();

		pTable->UpdateTableState(cardNum, cardType);
	}

}

void SevenGameMain::SetCurrentTurn(BasePacket* pPacket)
{
	SFProtobufPacket<SevenGamePacket::CurrentTurn>* pCurrentTurn = (SFProtobufPacket<SevenGamePacket::CurrentTurn>*)pPacket;

	m_SevenGameManger->m_iCurrentTurn = pCurrentTurn->GetData().playerindex();

}

void SevenGameMain::TurnPass(BasePacket* pPacket)
{
	SFProtobufPacket<SevenGamePacket::TurnPass>* pTurnPass = (SFProtobufPacket<SevenGamePacket::TurnPass>*)pPacket;
	int playerIndex = pTurnPass->GetData().playerindex();
	int ticketCnt = pTurnPass->GetData().ticketcount();

	SGUser* pUser = m_SevenGameManger->FindUser(playerIndex);
	pUser->DecreasePassTicketNum();
}

void SevenGameMain::CardSubmit(BasePacket* pPacket)
{
	SFProtobufPacket<SevenGamePacket::CardSubmit>* pCardSubmit = (SFProtobufPacket<SevenGamePacket::CardSubmit>*)pPacket;
	int cardNum = pCardSubmit->GetData().cardnum();
	int cardType = pCardSubmit->GetData().cardtype();
	int playerIndex = pCardSubmit->GetData().playerindex();

	SGUser* pUser = m_SevenGameManger->FindUser(playerIndex);
	
	pUser->EliminateCard(cardNum, cardType);

	SGTable* pTable = m_SevenGameManger->GetTable();
	pTable->UpdateTableState(cardNum, cardType);
}

void SevenGameMain::UserDie(BasePacket* pPacket)
{
	SFProtobufPacket<SevenGamePacket::UserDie>* pUserDie = (SFProtobufPacket<SevenGamePacket::UserDie>*)pPacket;
	int playerIndex = pUserDie->GetData().playerindex();

	SGUser* pUser = m_SevenGameManger->FindUser(playerIndex);
	pUser->SetRemainCard(0);
}

void SevenGameMain::OnWinner(BasePacket* pPacket)
{
	SFProtobufPacket<SevenGamePacket::Winner>* pWinner = (SFProtobufPacket<SevenGamePacket::Winner>*)pPacket;
	int playerIndex = pWinner->GetData().playerindex();

	if(playerIndex == m_SevenGameManger->GetMyID())
	{
		m_SevenGameManger->m_bIsVictory = TRUE;
		
	}
	else
	{
		m_SevenGameManger->m_bIsVictory = FALSE;
	}

	m_SevenGameManger->ChangeState(ENUM_SGGAMEOVER);
}