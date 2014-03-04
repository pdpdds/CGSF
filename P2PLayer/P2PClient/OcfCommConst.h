////////////////////////////////////////////////////////////////////////////////
//
// 작성자 : 이용휴
// 작성일 : 2007.11.01
// 설  명 :
//

#ifndef __OCFCOMMCONST_H__
#define __OCFCOMMCONST_H__
#ifndef __COMMCLIENT_H__
#define __COMMCLIENT_H__

////////////////////////////////////////////////////////////////////////////////
//
#define MAKE_OCFERROR(c)							(MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, c))

////////////////////////////////////////////////////////////////////////////////
//
// ERROR CATEGORY
//
typedef enum
{
	OCFCLIENT_ERROR_WINSOCK = 0,
	OCFCLIENT_ERROR_WIN32 = 1,
	OCFCLIENT_ERROR_USERDEF = 2
} OCFCLIENT_ERROR_CATEGORY;

////////////////////////////////////////////////////////////////////////////////
//
// ERROR CATEGORY - ERROR CODE
//
typedef enum
{
	OCFCLIENT_ERROR_USERDEF_CATEGORY_MEMORY = 1,
	OCFCLIENT_ERROR_USERDEF_CATEGORY_PKTMAXSIZE = 2,
	OCFCLIENT_ERROR_USERDEF_CATEGORY_RESET_CONNECTION = 255
} OCFCLIENT_ERROR_USERDEF_CATEGORY;

#endif
#endif

