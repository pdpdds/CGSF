#include "stdafx.h"
#include "PersonalInfo.h"
PersonalInfoManager::PersonalInfoManager()
{
}
PersonalInfoManager::~PersonalInfoManager()
{
	userPool.clear();
}
PersonalInfoManager* PersonalInfoManager::Ins()
{
	if (ins == nullptr)
		ins = new PersonalInfoManager();
	return ins;
}
int PersonalInfoManager::isNewUser(string sID)
{
	for (auto it = userPool.begin(); it != userPool.end(); it++)
	{
		if (it->sId.compare(sID) == 0)
			return 1;
	}
	return 0;
}
int PersonalInfoManager::tryLogin(std::string sID, std::string sPwd)
{
	for (auto it = userPool.begin(); it != userPool.end(); it++)
	{
		if (it->sId.compare(sID) == 0)
		{
			if(it->sPwd.compare(sPwd) == 0)
			{
				return 0;
			}
		}
	}
	return 1;
}
bool PersonalInfoManager::addUser(UserInfo info)
{
	if (isNewUser(info.sId) == 1)
		return false;
	userPool.push_back(info);
	return true;
}