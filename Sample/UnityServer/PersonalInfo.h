#pragma once
#include <list>
#include "CommonData.h"


class PersonalInfoManager
{
public:
	static PersonalInfoManager* Ins();
	PersonalInfoManager();
	virtual ~PersonalInfoManager();
	int isNewUser(std::string sID);
	int tryLogin(std::string sID, std::string sPwd);
	bool addUser(UserInfo info);
private:
	static PersonalInfoManager* ins;
	std::list<UserInfo> userPool;
};