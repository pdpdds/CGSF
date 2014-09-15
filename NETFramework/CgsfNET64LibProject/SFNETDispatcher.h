#pragma once
#include "ILogicDispatcher.h"

namespace CgsfNET64Lib {
	
	class SFNETDispatcher : public ILogicDispatcher
	{
	public:
		SFNETDispatcher(void);
		virtual ~SFNETDispatcher(void);

		void Init(int threadCount);

		virtual void Dispatch(BasePacket* pPacket) override;
		virtual bool CreateLogicSystem(ILogicEntry* pLogicEntry) override;
		virtual bool ShutDownLogicSystem() override;

		static bool ReleasePacket(BasePacket* pPacket);

	private:
		static bool m_bLogicEnd;
		int m_nLogicThreadCnt;

		static void LogicThreadProc(void* Args);
		
	};
}
