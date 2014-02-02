#pragma once 


const unsigned short PORT_NUMBER = 31400;

const int MAX_RECEIVE_BUFFER_LEN = 512;

const int MAX_NAME_LEN = 17;
const int MAX_MESSAGE_LEN = 129;




struct PACKET_HEADER
{
	short nID;
	short nSize;
};



//  ÆÐÅ¶
const short REQ_IN		= 1;
	// PKT_REQ_IN

const short RES_IN		= 2;
	// PKT_RES_IN

const short REQ_CHAT	= 6;
	// PKT_REQ_CHAT

const short NOTICE_CHAT = 7;
	// PKT_NOTICE_CHAT


struct PKT_REQ_IN : public PACKET_HEADER
{
	void Init()
	{
		nID = REQ_IN;
		nSize = sizeof(PKT_REQ_IN);
		memset( szName, 0, MAX_NAME_LEN );
	}

	char szName[MAX_NAME_LEN];
};

struct PKT_RES_IN : public PACKET_HEADER
{
	void Init()
	{
		nID = RES_IN;
		nSize = sizeof(PKT_RES_IN);
		bIsSuccess = false;
	}

	bool bIsSuccess;
};

struct PKT_REQ_CHAT : public PACKET_HEADER
{
	void Init()
	{
		nID = REQ_CHAT;
		nSize = sizeof(PKT_REQ_CHAT);
		memset( szMessage, 0, MAX_MESSAGE_LEN );
	}

	char szMessage[MAX_MESSAGE_LEN];
};

struct PKT_NOTICE_CHAT : public PACKET_HEADER
{
	void Init()
	{
		nID = NOTICE_CHAT;
		nSize = sizeof(PKT_NOTICE_CHAT);
		memset( szName, 0, MAX_NAME_LEN );
		memset( szMessage, 0, MAX_MESSAGE_LEN );
	}

	char szName[MAX_NAME_LEN]; 
	char szMessage[MAX_MESSAGE_LEN];
};