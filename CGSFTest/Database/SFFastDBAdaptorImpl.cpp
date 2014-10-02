#include "stdafx.h"
#include "SFFastDBAdaptorImpl.h"
#include "QueryIdentifier.h"
#include "fastdb.h"

class tbllogin { 
public:
	char const* szUsername;
	char const* szPassword;
public:
	TYPE_DESCRIPTOR((FIELD(szUsername), FIELD(szPassword)));
};

class tblBookInfo { 
public:
	char const* szName;
	char const* szAuthor;
	char const* szDesc;
public:
	TYPE_DESCRIPTOR((KEY(szName, HASHED|INDEXED), FIELD(szAuthor), FIELD(szDesc)));
};

class tblSongInfo { 
public:
	char const* szName;
	char const* szSinger;
	char const* szDesc;
public:
	TYPE_DESCRIPTOR((FIELD(szName), FIELD(szSinger), FIELD(szDesc)));
};

REGISTER(tblBookInfo);
REGISTER(tblSongInfo);
REGISTER(tbllogin);

SFFastDBAdaptorImpl::SFFastDBAdaptorImpl(void)
{
}


SFFastDBAdaptorImpl::~SFFastDBAdaptorImpl(void)
{
}

BOOL SFFastDBAdaptorImpl::RegisterDBService() 
{
	m_Dispatch.RegisterMessage(DBMSG_BOOKINFO, std::tr1::bind(&SFFastDBAdaptorImpl::OnBookInfo, this, std::tr1::placeholders::_1));
	m_Dispatch.RegisterMessage(DBMSG_SONGINFO, std::tr1::bind(&SFFastDBAdaptorImpl::OnSongInfo, this, std::tr1::placeholders::_1));
	return TRUE;
}

BOOL SFFastDBAdaptorImpl::OnBookInfo( BasePacket* pMessage )
{	
	char szName[256] = { 0, };

	SFMessage* pMsg = (SFMessage*)pMessage;

	*pMsg >> (char*)szName;

	dbQuery query;
	query = "szName=", szName;
	dbCursor<tblBookInfo>  BookInfo;

	if(BookInfo.select(query) > 0)
	{
		do 
		{ 
		//	printf("%s\t%s\t%s\n", BookInfo->szName, BookInfo->szAuthor, BookInfo->szDesc);

		} while (BookInfo.next());
	} 
	else 
	{ 
		printf("No Book Info\n");
	}

	return TRUE;
}

BOOL SFFastDBAdaptorImpl::OnSongInfo( BasePacket* pMessage )
{

	return TRUE;
}