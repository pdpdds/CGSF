#pragma once
#include <map>
#include "SFSubject.h"
#include "SFGameConstant.h"

class SFPlayer;

typedef std::map<UINT, SFPlayer*> TeamMemberMap;

class SFTeam : public SFSubject
{
public:
	SFTeam(void);
	virtual ~SFTeam(void);

	BOOL AddMember(SFPlayer* pPlayer);
	BOOL DelMember(SFPlayer* pPlayer);

	int GetMemberCount(){return m_MemberMap.size();}

	void SetTeamType(int TeamType){m_TeamType = (eTeamType)TeamType;}
	eTeamType GetTeamType(){return m_TeamType;}

	TeamMemberMap& GetMemberMap(){return m_MemberMap;}

protected:

private:
	TeamMemberMap m_MemberMap;
	eTeamType m_TeamType;
};