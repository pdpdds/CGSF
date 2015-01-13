#pragma once
#include "SFLogicDispatcher.h"
#include "SFIOCPQueue.h"
#include "BasePacket.h"
#include <map>

class SFMMODispatcher : public SFLogicDispatcher
{
	typedef std::map<int, SFIOCPQueue<BasePacket>*> mapQueue;
	typedef std::map<int, int> mapSerialChannel;
public:
	SFMMODispatcher(int channelCount = 1);
	virtual ~SFMMODispatcher();
	
	virtual void Dispatch(BasePacket* pPacket) override;

	virtual bool CreateLogicSystem(ILogicEntry* pLogicEntry) override;
	virtual bool ShutDownLogicSystem() override;

	mapQueue m_mapQueue;
	mapSerialChannel m_mapSerialChannel;

protected:

private:
	int m_channelCount;
	static void PacketDistributorProc(void* Args);
	static void MMOLogicProc(void* Args);
	static bool m_bLogicEnd;
};

