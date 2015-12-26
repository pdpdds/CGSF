#pragma once
class CommonProtocol
{
	/* 임의로 나눴다.
	* 시스템/테스트 패킷은 1001이하에 배정@
	* 요청-req패킷은 3만번대에 배정@
	* 응답-response패킷은 6만번대에 배정@
	*
	*/
public:
	const static USHORT RES_DISCONNECTED = 1; 
	const static USHORT REQ_ECHO = 1000;
	const static USHORT RET_ECHO = 1001;
	const static USHORT KEEPALIVE = 90000;///연결 유지@
	const static USHORT REQ_HELLo = 30000;
	const static USHORT RES_HELLo = 60000;


	const static USHORT REQ_LOGIN = 30001;///로그인 요청@
	const static USHORT REQ_IDISIDVALID = 30002;///아이디 중복확인@
	const static USHORT REQ_REGISTRATION = 30003;///신규 가입 요청@


	const static USHORT RES_LOGIN = 60001;///로그인 응답@
	const static USHORT RES_IDISIDVALID = 60002;///아이디 중복응답@
	const static USHORT RES_REGISTRATION = 60003;//신규 가입 응답@
	
};