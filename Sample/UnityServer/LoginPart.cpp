#include "stdafx.h"
#include "LoginPart.h"
#include "SFJsonPacket.h"
#include "CommonProtocol.h"
void LoginPart::REQD_LOGIN(BasePacket* pPacket)
{
	SFJsonPacket* pJsonPacket = (SFJsonPacket*)pPacket;//1. JsonPacket으로 받음@
	JsonObjectNode& req_Node = pJsonPacket->GetData();//2. iteration을 위한 node;
	
	CommonData::REQ_LoginInfo reqd_login;//3. 요청 패킷@
	reqd_login.sId = req_Node.GetValue<std::string>("sId");//3-1. json패킷에서 sID@
	reqd_login.sPwd = req_Node.GetValue<std::string>("sPwd");//3-2. json패킷에서 sPwd@	

	//CommonData::RES_LoginInfo res_login;//4. 응답 패킷@
	SFJsonPacket JsonPacket(CommonProtocol::RES_LOGIN);//4-1. json응답 패킷의 프로토콜@
	JsonObjectNode& res_Node = JsonPacket.GetData();//4-2. json 데이터를 넣기위한 node@
	
	int iPwdChk = PersonalInfoManager::Ins()->tryLogin(reqd_login.sId, reqd_login.sPwd);
	//요청 받은 아이디가 인증이 가능하다면@
	//예제용임. 실제는 탐색 결과를 받아 처리@
	if(iPwdChk == 0)
	{
		res_Node.Add("iResult", 0);//패스@
	}
	else if (iPwdChk == 1)
	{
		res_Node.Add("iResult", iPwdChk);//없는아이디@
	}
	else if (iPwdChk == 3)
	{
		res_Node.Add("iResult", iPwdChk);//꽉참@
	}
	else if (iPwdChk == 4)
	{
		res_Node.Add("iResult", iPwdChk);//알 수 없는 에러@
	}

	///이하 테스트 코드@
	if (reqd_login.sId.compare("foranie") ==0)
	{
		/*
		//원래 들어가야 할 내용@
		res_login.iResult = 0;
		res_login.iWorldCount = 2;
		res_login.sIp = new std::string[2];
		res_login.sIp[0] = "127.0.0.1";
		res_login.sIp[1] = "127.0.0.1";
		*/
		res_Node.Add("iResult", 0);//로그인 통과@
		res_Node.Add("iWorldCount", 2);
		std::cout << "req_login result : 0" << endl;
	}
	else if (reqd_login.sId.compare("noID") == 0)
	{
		res_Node.Add("iResult", 1);//아이디 없음 결과 반환@
		std::cout << "req_login result : 1" << endl;
	}
	else if (reqd_login.sId.compare("serverFULL") == 0)
	{
		res_Node.Add("iResult", 3);//서버꽉참 결과 반환@
		std::cout << "req_login result : 3" << endl;
	}
	else if (reqd_login.sId.compare("etcError") == 0)
	{
		res_Node.Add("iResult", 4);//기타 이유@
		std::cout << "req_login result : 4" << endl;
	}
	///여기까지 테스트 코드@
	JsonPacket.SetSerial(pPacket->GetSerial());
	SFEngine::GetInstance()->SendRequest(&JsonPacket);
}
void LoginPart::REQD_IDISIDVALID(BasePacket* pPacket)
{
	SFJsonPacket* pJsonPacket = (SFJsonPacket*)pPacket;//1. JsonPacket으로 받음@
	JsonObjectNode& req_Node = pJsonPacket->GetData();//2. iteration을 위한 node;

	std::string sId = req_Node.GetValue<std::string>("sId");
	int iNew = PersonalInfoManager::Ins()->isNewUser(sId);

	SFJsonPacket JsonPacket(CommonProtocol::RES_IDISIDVALID);//4-1. json응답 패킷의 프로토콜@
	JsonObjectNode& res_Node = JsonPacket.GetData();//4-2. json 데이터를 넣기위한 node@
	
	if (iNew == 0)//새 아이디@
	{
		res_Node.Add("iResult", 0);
	}
	else if (iNew == 1)//중복@
	{
		res_Node.Add("iResult", 1);
	}

	JsonPacket.SetSerial(pPacket->GetSerial());
	SFEngine::GetInstance()->SendRequest(&JsonPacket);
}
void LoginPart::REQD_REGISTRATION(BasePacket* pPacket)
{
	SFJsonPacket* pJsonPacket = (SFJsonPacket*)pPacket;//1. JsonPacket으로 받음@
	JsonObjectNode& req_Node = pJsonPacket->GetData();//2. iteration을 위한 node;
	
	CommonData::REQ_Registration reqd_reg;//3. 요청 패킷@
	reqd_reg.sId = req_Node.GetValue<std::string>("sId");//3-1. json패킷에서 sID@
	reqd_reg.sPwd = req_Node.GetValue<std::string>("sPwd");//3-2. json패킷에서 sPwd@
	reqd_reg.sMail = req_Node.GetValue<std::string>("sMail");//3-3. json패킷에서 sMail@
	reqd_reg.sExtra = req_Node.GetValue<std::string>("sExtra");//3-4. json패킷에서 sExtra@
	reqd_reg.sUDID = req_Node.GetValue<std::string>("sUDID");//3-5. json패킷에서 sUDID@

	SFJsonPacket JsonPacket(CommonProtocol::RES_REGISTRATION);//4-1. json응답 패킷의 프로토콜@
	JsonObjectNode& res_Node = JsonPacket.GetData();//4-2. json 데이터를 넣기위한 node@
	
	bool bAddUser = PersonalInfoManager::Ins()->addUser(reqd_reg);
	if (bAddUser == true)
	{
		res_Node.Add("iResult", 0);
	}
	else
	{
		res_Node.Add("iResult", 2);
	}

	JsonPacket.SetSerial(pPacket->GetSerial());
	SFEngine::GetInstance()->SendRequest(&JsonPacket);
}
