////////////////////////////////////////////////////////////////////////////////
// 서브시스템  : 
// 프로그램 ID : Common.h
// 기능요약    : 시스템 전반에 사용되는 상수 및 구조체 정의..
// 작성일      : 2001.03.19
// 작성자      : 이용휴
// 수정일      :
// 수정자      :
// 수정내역    :
////////////////////////////////////////////////////////////////////////////////

#ifndef __COMMON_H__
#define __COMMON_H__

////////////////////////////////////////////////////////////////////////////////
//
#define MAX_TMPBUFF_LEN			256
#define MAX_MSGBUFF_LEN			1024

#if defined(PATH_MAX) && PATH_MAX <= 1024
#define MAX_PATH_LEN			1024
#else
//////////////////////////////////////////////////////////////////////////
// 1024가 넘는 것은 강제로 다시 설정...
//
#define MAX_PATH_LEN			1024
#endif

////////////////////////////////////////////////////////////////////////////////
//
enum QUESTION
{
	NO = 0,
	YES
};

////////////////////////////////////////////////////////////////////////////////
//
enum THREAD_STATUS
{
	THREAD_STARTING = 1,
	THREAD_RUNNING = 2,
	THREAD_ENDING = 3,
	THREAD_ZOMBI = 4
};

////////////////////////////////////////////////////////////////////////////////
//
enum EVENT_TYPE
{
	EVENT = 1,
	EVENT_SOCK = 2
};

////////////////////////////////////////////////////////////////////////////////
//
typedef enum _FD_STATUS
{
// 	FD_CONNECT = 1,
// 	FD_RETRYCONNECT = 2,
// 	FD_DISCONNECT = 3
} FD_STATUS;

////////////////////////////////////////////////////////////////////////////////
//
//#ifdef __USE_BIG_ENDIAN__
#define B_STOINT(s)				(((s[0]<<8)&0xff00)|(s[1]&0x00ff))
#define B_STOLONG(s)			(((s[0]<<24)&0xff000000)| \
								 ((s[1]<<16)&0x00ff0000)| \
								 ((s[2]<<8)&0x0000ff00)| \
								  (s[3]&0x000000ff))
#define B_INTTOS(i,s)			{s[1]=(unsigned char)(i&0xff); \
								 s[0]=(unsigned char)((i>>8)&0xff);}
#define B_LONGTOS(l,s)			{s[3]=(unsigned char)(l&0xff); \
								 s[2]=(unsigned char)((l>>8)&0xff); \
								 s[1]=(unsigned char)((l>>16)&0xff); \
								 s[0]=(unsigned char)((l>>24)&0xff);}
//#else
#define L_STOINT(s)				(((s[1]<<8)&0xff00)|(s[0]&0x00ff))
#define L_STOLONG(s)			(((s[3]<<24)&0xff000000)| \
								 ((s[2]<<16)&0x00ff0000)| \
								 ((s[1]<<8)&0x0000ff00)| \
								  (s[0]&0x000000ff))
#define L_INTTOS(i,s)			{s[0]=(unsigned char)(i&0xff); \
								 s[1]=(unsigned char)((i>>8)&0xff);}
#define L_LONGTOS(l,s)			{s[0]=(unsigned char)(l&0xff); \
								 s[1]=(unsigned char)((l>>8)&0xff); \
								 s[2]=(unsigned char)((l>>16)&0xff); \
								 s[3]=(unsigned char)((l>>24)&0xff);}
//#endif
#endif


