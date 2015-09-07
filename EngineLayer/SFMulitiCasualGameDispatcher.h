#pragma once
#include "SFLogicDispatcher.h"
#include "SFIOCPQueue.h"
#include "BasePacket.h"
#include <vector>

class SFMulitiCasualGameDispatcher : public SFLogicDispatcher
{
	typedef std::vector<SFIOCPQueue<BasePacket>*> vecQueue;

public:
	SFMulitiCasualGameDispatcher(int channelCount = 1);
	virtual ~SFMulitiCasualGameDispatcher();
	
	virtual void Dispatch(BasePacket* pPacket) override;	
	
	virtual bool OnAuthenticate(BasePacket* pPacket) = 0;

	virtual bool CreateLogicSystem(ILogicEntry* pLogicEntry) override;
	virtual bool ShutDownLogicSystem() override;

	bool DistributePacket(BasePacket* pPacket);

	vecQueue m_vecQueue;

	int GetMaxChannelCount() { return m_channelCount; }

protected:

private:
	int m_channelCount;	
	static void CasualGameLogicProc(void* Args);
	static bool m_bLogicEnd;
};

