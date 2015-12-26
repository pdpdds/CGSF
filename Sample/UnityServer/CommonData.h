#pragma once
#include <string>
namespace CommonData
{
	//요청 패킷 3만번대@
	///@brief : 로그인 요청@
	class REQ_LoginInfo
	{
	public:
		std::string sId;
		std::string sPwd;
	};
	///@brief : 아이디 중복 확인@
	class REQ_IdIsValid
	{
	public:
		std::string sId;
	};
	///@brief : 가입 요청@
	class REQ_Registration
	{
	public:
		std::string sId;
		std::string sPwd;
		std::string sMail;
		std::string sExtra;
		std::string sUDID;
	};
	#define UserInfo CommonData::REQ_Registration

	//응답 패킷 6만번대@
	class RES
	{///모든 응답 패킷은 결과값을 가진다@
	public:
		int iResult = -1;
	};
	////////////
	class RES_LoginInfo : public RES
	{/// = 0 : 성공 아이피등 전송 받음, = 1 :실패@
	public:
		int iWorldCount;
		std::string* sIp;
	};
	class RES_IdIsValid : public RES
	{///RES = 0 : 성공, = 1 : 사용 불가, = 2 : 중복아이디@
	};
	class RES_Regstration : public RES
	{
	};
}