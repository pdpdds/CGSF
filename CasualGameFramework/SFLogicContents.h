#pragma once
#include <list>

class SFLogicContents
{
	typedef std::list<int> SpawnList;
public:
	SFLogicContents(void);
	virtual ~SFLogicContents(void);

	BOOL Load();

protected:
	BOOL LoadSpawn();

private:
	SpawnList m_SpawnList;
};
