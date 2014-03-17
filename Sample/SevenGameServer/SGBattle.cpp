#include "stdafx.h"
#include "SGBattle.h"
#include "SGManager.h"
#include "SFRoom.h"
#include "SFRoomPlay.h"
#include "Macro.h"
#include "SFPlayer.h"
#include "BasePacket.h"
#include "SFProtobufPacket.h"
#include "SFEngine.h"
#include "SGUser.h"
#include "SGTable.h"
#include "GameModeConstant.h"

SGBattle::SGBattle(int Mode)
: SFGameMode(Mode)
{
	m_DispatchingSystem.RegisterMessage(SevenGame::TurnPass, std::tr1::bind(&SGBattle::OnTurnPass, this, std::tr1::placeholders::_1, std::tr1::placeholders::_2));
	m_DispatchingSystem.RegisterMessage(SevenGame::CardSubmit, std::tr1::bind(&SGBattle::OnCardSubmit, this, std::tr1::placeholders::_1, std::tr1::placeholders::_2));

	m_SevenGameManger = new SGManager();
	m_SevenGameManger->AllocateObjcet(SEVENGAME_MEMBER_NUM, MAX_PASS_TICKET); // create four User, 3 pass ticket!!
}

SGBattle::~SGBattle(void)
{
	delete m_SevenGameManger;
	m_SevenGameManger = NULL;
}


#define SG_GAME_PLAY  1
#define SG_GAME_GAMEOVER  2
#define SG_DELAY_GAMEOVER  3000
#define SG_AI_WAIT_TIME 800
#define SG_PLAYER_WAIT_TIME 10000

BOOL SGBattle::OnEnter( int GameMode )
{
	SFRoom* pRoom = GetOwner()->GetOwner();
	SFRoom::RoomMemberMap& roomMember = pRoom->GetRoomMemberMap();

	m_GameState = SG_GAME_PLAY;
	m_dwLastPlayTickCount = GetTickCount();

	SFASSERT(roomMember.size() != 0 && roomMember.size() <= SEVENGAME_MEMBER_NUM);

	m_SevenGameManger->Reset();

	int playerCount = 0;
	for (auto& iter : roomMember)
	{
		SFPlayer* pPlayer = iter.second;
		m_SevenGameManger->AddUser(pPlayer->GetSerial());

		playerCount++;
	}

	int AISerial = -1;
	while(playerCount < SEVENGAME_MEMBER_NUM)
	{
		m_SevenGameManger->AddUser(AISerial);
		AISerial--;
		playerCount++;
	}

	ResetGame();

	return TRUE;
}

void SGBattle::ResetGame()
{
	m_SevenGameManger->InitializeData();
	m_SevenGameManger->PlayerShuffle();

	SendPlayerID();
	SendInitCardCount();
	SendMyCardInfo();
	SendTableUpdate();

	vector<SGUser*>::iterator pos = m_SevenGameManger->m_userlist.begin();

	m_SevenGameManger->SetCurrentUserID((*pos)->GetID());

	SendCurrentTurn();
}

BOOL SGBattle::Onleave()
{
	return TRUE;
}

BOOL SGBattle::Update( DWORD dwTickcount )
{
	if(m_GameState ==  SG_GAME_PLAY)
	{
		int playerID = m_SevenGameManger->GetCurrentUserID();

		if(playerID < 0) //AI
		{
			if(GetTickCount() > m_dwLastPlayTickCount + SG_AI_WAIT_TIME)
			{
				SGUser* pAI = m_SevenGameManger->FindUser(playerID);
				SFASSERT(pAI != NULL);
				SCardInfo cardInfo = m_SevenGameManger->ProcessAI(pAI);
				if(cardInfo.iCardNum == -1)
				{
					SendTurnPass(pAI->GetID());
				}
				else
				{
					m_SevenGameManger->UpdateTableState(&cardInfo);
					SendCardSubmit(pAI->GetID(), cardInfo);
				}

				m_SevenGameManger->SetCurrentUserID(m_SevenGameManger->GetNextUserID(pAI->GetID()));
				if(-1 == m_SevenGameManger->EvaluateUser(pAI))
				{
					SendUserDie(pAI->GetID());
				}

				if(FALSE == m_SevenGameManger->CheckGameEnd())
					SendCurrentTurn();
				else
				{
					if(m_SevenGameManger->CheckGameEnd())
					{
						SendWinner();
						m_GameState = SG_GAME_GAMEOVER;
						m_dwLastTickCount = GetTickCount();
						return TRUE;
					}
				}
			}
		}
		else
		{
			if(GetTickCount() > m_dwLastPlayTickCount + SG_PLAYER_WAIT_TIME)
			{
				ProcessTurnPass(playerID);
			}
		}
	}
	else if (m_GameState == SG_GAME_GAMEOVER)
	{
		DWORD dwTickCount = GetTickCount();

		if(dwTickCount - m_dwLastTickCount> SG_DELAY_GAMEOVER)
		{
			m_GameState = SG_GAME_PLAY;
			ResetGame();
		}

	}

	return TRUE;
}

BOOL SGBattle::ProcessUserRequest( SFPlayer* pPlayer, int Msg )
{
	return TRUE;
}

BOOL SGBattle::ProcessUserRequest( SFPlayer* pPlayer, BasePacket* pPacket )
{
	return m_DispatchingSystem.HandleMessage(pPacket->GetPacketID(), pPlayer, pPacket);
	return TRUE;
}

BOOL SGBattle::OnTurnPass( SFPlayer* pPlayer, BasePacket* pPacket)
{	
	SFProtobufPacket<SevenGamePacket::TurnPass>* pPass = (SFProtobufPacket<SevenGamePacket::TurnPass>*)pPacket;
	int currentTurnUser = m_SevenGameManger->GetCurrentUserID();

	if(currentTurnUser != pPlayer->GetSerial())
		return FALSE;

	return ProcessTurnPass(currentTurnUser);
}

BOOL SGBattle::ProcessTurnPass(int currentTurnUser)
{
	SGUser* pUser = m_SevenGameManger->FindUser(currentTurnUser);

	if(FALSE == m_SevenGameManger->ProcessTurnPass(currentTurnUser))
		return FALSE;

	SendTurnPass(currentTurnUser);

	m_SevenGameManger->SetCurrentUserID(m_SevenGameManger->GetNextUserID(pUser->GetID()));
	int result = m_SevenGameManger->EvaluateUser(pUser);

	if(result == -1)
	{
		SendUserDie(currentTurnUser);
	}

	if(m_SevenGameManger->CheckGameEnd())
	{
		SendWinner();
		m_GameState = SG_GAME_GAMEOVER;
		m_dwLastTickCount = GetTickCount();

	}
	else
	{
		SendCurrentTurn();
	}

	return TRUE;
}

BOOL SGBattle::OnCardSubmit( SFPlayer* pPlayer, BasePacket* pPacket)
{
	SFProtobufPacket<SevenGamePacket::CardSubmit>* pCardSubmit = (SFProtobufPacket<SevenGamePacket::CardSubmit>*)pPacket;
	int currentTurnUser = m_SevenGameManger->GetCurrentUserID();
	SGUser* pUser = m_SevenGameManger->FindUser(currentTurnUser);

	if(currentTurnUser != pPlayer->GetSerial())
		return FALSE;

	SCardInfo info;
	info.iCardNum = pCardSubmit->GetData().cardnum();
	info.iCardType = pCardSubmit->GetData().cardtype();

	if(FALSE == m_SevenGameManger->ProcessCardSubmit(currentTurnUser, info))
		return FALSE;

	m_SevenGameManger->UpdateTableState(&info);

	SendCardSubmit(currentTurnUser, info);
	
	m_SevenGameManger->SetCurrentUserID(m_SevenGameManger->GetNextUserID(pUser->GetID()));
	m_SevenGameManger->EvaluateUser(pUser);

	if(m_SevenGameManger->CheckGameEnd())
	{
		SendWinner();
		m_GameState = SG_GAME_GAMEOVER;
		m_dwLastTickCount = GetTickCount();
		return TRUE;
	}
	else
	{
		SendCurrentTurn();
	}

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL SGBattle::SendPlayerID()
{
		SFRoom* pRoom = GetOwner()->GetOwner();
	SFRoom::RoomMemberMap& roomMember = pRoom->GetRoomMemberMap();

	SFRoom::RoomMemberMap::iterator iter = roomMember.begin();
	
	for(;iter != roomMember.end(); iter++)
	{
		SFPlayer* pPlayer = iter->second;
		
		SFProtobufPacket<SevenGamePacket::PlayerID> packet(SevenGame::PlayerID);
		packet.SetOwnerSerial(pPlayer->GetSerial());
		packet.GetData().set_playerindex(pPlayer->GetSerial());

		SFEngine::GetInstance()->SendRequest(&packet);
	}
	

	return TRUE;
}

BOOL SGBattle::SendInitCardCount()
{
	SFRoom* pRoom = GetOwner()->GetOwner();
	SFRoom::RoomMemberMap& roomMember = pRoom->GetRoomMemberMap();

	SFProtobufPacket<SevenGamePacket::InitCardCount> initCardCount(SevenGame::InitCardCount);

	vector<SGUser*>::iterator pos  = m_SevenGameManger->m_userlist.begin();

	while ( pos != m_SevenGameManger->m_userlist.end()) 	 			
	{
		SGUser* pUser = (SGUser*)(*pos);

		SevenGamePacket::InitCardCount::CardCount* pCardCount = initCardCount.GetData().add_info();
			
		pCardCount->set_playerindex(pUser->GetID());
		pCardCount->set_cardcount(pUser->GetRemainCard());

		pos++;		
	}

	BroadCast(initCardCount);

	return TRUE;
}

BOOL SGBattle::SendMyCardInfo()
{
	SFRoom* pRoom = GetOwner()->GetOwner();
	SFRoom::RoomMemberMap& roomMember = pRoom->GetRoomMemberMap();

	SFRoom::RoomMemberMap::iterator iter = roomMember.begin();
	
	for(;iter != roomMember.end(); iter++)
	{
		SFProtobufPacket<SevenGamePacket::MyCardInfo> myCardInfo(SevenGame::MyCardInfo);

		SFPlayer* pPlayer = iter->second;
		myCardInfo.SetOwnerSerial(pPlayer->GetSerial());

		SGUser* pSGUser = m_SevenGameManger->FindUser(pPlayer->GetSerial());

		SFASSERT(pSGUser != NULL);

		int* pArray = pSGUser->GetSpadeArray();
		int CardCount = 0;
		for( int i=1 ; i<= MAX_CARD_NUM ; i++)
		{
			if( pArray[i] != -1)
			{
				SevenGamePacket::MyCardInfo::PlayerCard* pPlayerCard = myCardInfo.GetData().add_card();
				pPlayerCard->set_cardnum(i);
				pPlayerCard->set_cardtype(TYPE_SPADE);
				CardCount++;
				printf("%d %d\n", i, TYPE_SPADE);
			}
		}

		pArray = pSGUser->GetHeartArray();

		for( int i=1 ; i<= MAX_CARD_NUM ; i++)
		{
			if( pArray[i] != -1)
			{
				SevenGamePacket::MyCardInfo::PlayerCard* pPlayerCard = myCardInfo.GetData().add_card();
				pPlayerCard->set_cardnum(i);
				pPlayerCard->set_cardtype(TYPE_HEART);
				CardCount++;
				printf("%d %d\n", i, TYPE_HEART);
			}
		}

		pArray = pSGUser->GetDiamondArray();

		for( int i=1 ; i<= MAX_CARD_NUM ; i++)
		{
			if( pArray[i] != -1)
			{
				SevenGamePacket::MyCardInfo::PlayerCard* pPlayerCard = myCardInfo.GetData().add_card();
				pPlayerCard->set_cardnum(i);
				pPlayerCard->set_cardtype(TYPE_DIAMOND);
				CardCount++;
					printf("%d %d\n", i, TYPE_DIAMOND);
			}
		}

		pArray = pSGUser->GetCloverArray();

		for( int i=1 ; i<= MAX_CARD_NUM ; i++)
		{
			if( pArray[i] != -1)
			{
				SevenGamePacket::MyCardInfo::PlayerCard* pPlayerCard = myCardInfo.GetData().add_card();
				pPlayerCard->set_cardnum(i);
				pPlayerCard->set_cardtype(TYPE_CLOVER);
				CardCount++;
				printf("%d %d\n", i, TYPE_CLOVER);
			}
		}
					
		SFASSERT(pSGUser->GetRemainCard() == CardCount);
		SFEngine::GetInstance()->SendRequest(&myCardInfo);
	}

	return TRUE;
}

BOOL SGBattle::BroadCast(BasePacket& packet)
{
	SFRoom* pRoom = GetOwner()->GetOwner();
	SFRoom::RoomMemberMap& roomMember = pRoom->GetRoomMemberMap();

	SFRoom::RoomMemberMap::iterator iter = roomMember.begin();
	
	for(;iter != roomMember.end(); iter++)
	{
		SFPlayer* pPlayer = iter->second;
		packet.SetOwnerSerial(pPlayer->GetSerial());

		SFEngine::GetInstance()->SendRequest(&packet);
	}

	return TRUE;
}

void SGBattle::SendCurrentTurn()
{
	SFRoom* pRoom = GetOwner()->GetOwner();
	SFRoom::RoomMemberMap& roomMember = pRoom->GetRoomMemberMap();

	int currentUserID = m_SevenGameManger->GetCurrentUserID();
	SGUser* pSGUser = m_SevenGameManger->FindUser(currentUserID);

	SFProtobufPacket<SevenGamePacket::CurrentTurn> currentTurn(SevenGame::CurrentTurn);

	if (pSGUser->GetID() <= 0)
	{
		currentTurn.GetData().set_playerindex(pSGUser->GetID());
		currentTurn.GetData().set_playername("AI");
	}
	else
	{
		SFRoom::RoomMemberMap::iterator iter = roomMember.find(pSGUser->GetID());

		SFASSERT(iter != roomMember.end());
		SFPlayer* pCurrentTurnPlayer = iter->second;

		currentTurn.GetData().set_playerindex(pCurrentTurnPlayer->GetSerial());
		currentTurn.GetData().set_playername(pCurrentTurnPlayer->GetPlayerName());
	}

	m_dwLastPlayTickCount = GetTickCount();
	BroadCast(currentTurn);
}

void SGBattle::SendTurnPass(int PlayerIndex)
{
	SGUser* pUser = m_SevenGameManger->FindUser(PlayerIndex);

	SFProtobufPacket<SevenGamePacket::TurnPass> turnPass(SevenGame::TurnPass);
	turnPass.GetData().set_playerindex(PlayerIndex);
	turnPass.GetData().set_ticketcount(pUser->GetPassTicketCnt());
	BroadCast(turnPass);
}

void SGBattle::SendCardSubmit(int PlayerIndex, SCardInfo& cardInfo)
{
	SFProtobufPacket<SevenGamePacket::CardSubmit> cardSubmit(SevenGame::CardSubmit);
	cardSubmit.GetData().set_playerindex(PlayerIndex);
	cardSubmit.GetData().set_cardnum(cardInfo.iCardNum);
	cardSubmit.GetData().set_cardtype(cardInfo.iCardType);
	BroadCast(cardSubmit);
}

void SGBattle::SendWinner()
{
	SFProtobufPacket<SevenGamePacket::Winner> winner(SevenGame::Winner);

	winner.GetData().set_playerindex(*(m_SevenGameManger->m_vecWinner.begin()));

	BroadCast(winner);
}

void SGBattle::SendUserDie(int playerIndex)
{
	SendTableUpdate();

	SFProtobufPacket<SevenGamePacket::UserDie> userDie(SevenGame::UserDie);

	userDie.GetData().set_playerindex(playerIndex);

	BroadCast(userDie);
}

void SGBattle::SendTableUpdate()
{
	SFProtobufPacket<SevenGamePacket::TableUpdate> tableUpdate(SevenGame::TableUpdate);

	SGTable* pTable = m_SevenGameManger->GetTable();

	int *paSpadeTable = pTable->GetSpadeTableArray();
	int *paHeartTable = pTable->GetHeartTableArray();
	int *paCloverTable = pTable->GetCloverTableArray();
	int *paDiamondTable = pTable->GetDiamondTableArray();

	for(int i=1; i<=MAX_CARD_NUM; i++ )
	{
		if(paSpadeTable[i] != -1)	   
		{
			SevenGamePacket::TableUpdate::AddCard* pCard = tableUpdate.GetData().add_card();
			pCard->set_cardnum(paSpadeTable[i]);
			pCard->set_cardtype(TYPE_SPADE);
		}  								
	}

	for(int i=1; i<=MAX_CARD_NUM; i++ )
	{
		if(paHeartTable[i] != -1)	   
		{
			SevenGamePacket::TableUpdate::AddCard* pCard = tableUpdate.GetData().add_card();
			pCard->set_cardnum(paHeartTable[i]);
			pCard->set_cardtype(TYPE_HEART);
		}  								
	}

	for(int i=1; i<=MAX_CARD_NUM; i++ )
	{
		if(paCloverTable[i] != -1)	   
		{
			SevenGamePacket::TableUpdate::AddCard* pCard = tableUpdate.GetData().add_card();
			pCard->set_cardnum(paCloverTable[i]);
			pCard->set_cardtype(TYPE_CLOVER);
		}  								
	}

	for(int i=1; i<=MAX_CARD_NUM; i++ )
	{
		if(paDiamondTable[i] != -1)	   
		{
			SevenGamePacket::TableUpdate::AddCard* pCard = tableUpdate.GetData().add_card();
			pCard->set_cardnum(paDiamondTable[i]);
			pCard->set_cardtype(TYPE_DIAMOND);
		}  								
	}

	BroadCast(tableUpdate);
}