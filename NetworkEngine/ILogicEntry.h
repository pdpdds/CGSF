#pragma once
#include <map>

class SFCommand;

class ILogicEntry
{
public:
	typedef std::map<DWORD, _TimerInfo> TimerMap;

	ILogicEntry(void){}
	virtual ~ILogicEntry(void){}

	virtual BOOL ProcessPacket(SFCommand* pCommand) = 0;

	BOOL AddTimer(_TimerInfo Info)
	{
		m_TimerMap.insert(std::make_pair(Info.TimerID, Info));

		return TRUE;
	}

	TimerMap& GetTimerMap(){return m_TimerMap;}

protected:

private:
	TimerMap m_TimerMap;
};
