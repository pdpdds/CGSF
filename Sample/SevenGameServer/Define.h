#pragma once

typedef struct
{
	int Low_Card;
	int High_Card;
}EdgeCard;

typedef struct
{
	int iCardType;
	int iCardNum;
}SCardInfo;

#define GAME_RUNNING    0
#define GAME_END        1

#define MAX_CARD_NUM 13
#define MAX_PASS_NUM 3

#define TYPE_CLOVER  0
#define TYPE_HEART   1
#define TYPE_DIAMOND 2
#define TYPE_SPADE   3

#define PRIORTY_A    0
#define PRIORTY_B	 1
#define PRIORTY_C	 2
#define PRIORTY_D	 3
#define PRIORTY_E	 4
#define PRIORTY_F	 5

#define ERROR_TABLE_NULL   0
#define ERROR_USER_NULL    1
#define CARD_PASS_OK       2
#define CARD_PASS_NO       3

#define MAX_ALL_CARD_NUM 52;


// TODO: reference additional headers your program requires here
#define LOCK_TIME_OUT				1000*20
#include <string>
#include <map>
#include <vector>
#include <list>
using namespace std;

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#endif
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
#endif
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }
#endif

#define LOCK_CRITICAL_SECTION		// Critical Section 을 쓴다.
#ifdef LOCK_CRITICAL_SECTION 
#define LOCK_RESOURCE(X)	 	CLockCritical	X;

#else
#define LOCK_RESOURCE(X)		CLockEvent		X;		
#endif


typedef struct SPRITE_CARD_INFO
{
	UINT iCardType;
	BOOL bCardVisible;
	RECT CurrentRect;
	RECT OriginalRect;
	BOOL bCardMove;
	int  iCol;
	int  iRow;
}SPRITE_CARD_INFO;

#define LEN_GUI_CAPTION			100

typedef struct tagComponentInfo
{
	int PannelID;
	int iPosX;
	int iPosY;
	int iHeight;
	int iWidth;
	int iComponentType;
	int iIdentifier;
	TCHAR szGUICaption[LEN_GUI_CAPTION];

	/////////////////////////////////////////////////////
	//For Radio Button
	/////////////////////////////////////////////////////
	int iButtonGroup;
	bool bDefault;

	tagComponentInfo()
	{
		PannelID = -1;
		iComponentType = -1;
		iIdentifier = -1;
		iPosX = -1;
		iPosY = -1;
		iHeight = -1;
		iWidth = -1;
		memset(szGUICaption, 0, sizeof(TCHAR) * LEN_GUI_CAPTION);

		iButtonGroup = -1;
		bDefault = false;
	}

}ComponentInfo;

typedef struct tag_CARD_EFFECT_INFO
{
	int iDestX;
	int iDestY;
	int iOriX;
	int iOriY;
}CARD_EFFECT_INFO;

enum eSGState
{
	ENUM_SGINIT,
	ENUM_SGCARDEFFECT,
	ENUM_SGCARDTURN,
	ENUM_SGGAME,
	ENUM_SGSTART,
	ENUM_SGGAMEOVER,
	ENUM_SGCARDPASSEFFECT,
};

#define PASSBUTTON_LEFT 180
#define PASSBUTTON_RIGHT 280
#define PASSBUTTON_TOP 680
#define PASSBUTTON_BOTTOM 740

#define CARD_WIDTH 58.0f
#define CARD_HEIGHT 81.0f

#define DEFAULT_TABLE_CARD_POS_X  320 //바닥에 펼쳐지는 카드의 기준점
#define DEFAULT_TABLE_CARD_POS_Y  200

#define CARD_LOOKABLE_PART  28//카드가 겹쳐졌을 때 보이는 영역
#define CARD_DISTANCE  (CARD_HEIGHT + 1.0f)//각각의 카드 종류간의 거리

#define CARDNUM_CARDSET_WIDTH 13.0f //카드셋 이미지에서 열의 수
#define CARDNUM_CARDSET_HEIGHT 4.0f //카드셋 이미지에서 행의 수

#define CARDRATIO_CARDSET_WIDTH (1.0f / 13.0f) //카드셋 이미지에서 한 카드의 가로 비율
#define CARDRATIO_CARDSET_HEIGHT (1.0f / 4.0f) //카드셋 이미지에서 한 카드의 세로 비율

#define SILENCE_PLAYER_CARD_POS_X    0
#define SILENCE_PLAYER_CARD_POS_Y    360

#define DETECTIVE_PLAYER_POS_X   943
#define DETECTIVE_PLAYER_POS_Y   360

#define HUMAN_PLAYER_POS_X  472
#define HUMAN_PLAYER_POS_Y  681


#define GENIUS_PLAYER_POS_X    472
#define GENIUS_PLAYER_POS_Y    0

///////////////////////////////////////////////////////////////////////////////////////
//이미지
///////////////////////////////////////////////////////////////////////////////////////
#define PICTUREBOX_CARDSET 1
#define PICTUREBOX_CARDHORIZON 2
#define PICTUREBOX_VERTICAL 3
#define PICTUREBOX_BUTTON 4
#define PICTUREBOX_BUTTON_OVERPPED 5

#define PICTUREBOX_YOURTURN 6
#define PICTUREBOX_AITURN 7
#define PICTUREBOX_PASS 8
#define PICTUREBOX_YOUWIN 9
#define PICTUREBOX_YOULOSE 10
#define PICTUREBOX_GAMESTART 11
#define PICTUREBOX_NOPASS  12