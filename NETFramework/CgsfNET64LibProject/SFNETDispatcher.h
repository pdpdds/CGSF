#pragma once
#include "ILogicDispatcher.h"
#include "RPCService.h"

namespace CgsfNET64Lib {
	
	/// <summary>
	/// 네트워크 엔진에서 패킷을 가져온다.
	/// </summary>
	class SFNETDispatcher : public ILogicDispatcher
	{
	public:
		SFNETDispatcher(void);
		virtual ~SFNETDispatcher(void);

		/// <summary>
		/// 초기화. 스레드 수를 설정한다.
		/// </summary>
		/// <param name="threadCount">스레드 수</param>
		void Init(int threadCount);

		/// <summary>
		/// 패킷을 가져와서 큐에 담는다
		/// </summary>
		/// <param name="pPacket">패킷</param>
		virtual void Dispatch(BasePacket* pPacket) override;

		/// <summary>
		/// 패킷 디스패처를 할 스레드를 생성한다.
		/// </summary>
		/// <param name="pLogicEntry">패킷을 처리할 객체</param>
		/// <returns>무조건 true 반환</returns>
		virtual bool CreateLogicSystem(ILogicEntry* pLogicEntry) override;

		/// <summary>
		/// 패킷 디스패처를 종료한다. 보통 서버 종료 시에 호출한다.
		/// </summary>
		/// <returns>무조건 true 반환</returns>
		virtual bool ShutDownLogicSystem() override;

		/// <summary>
		/// 패킷 객체의 메모리 해제
		/// </summary>
		/// <param name="pPacket">메모리 해제할 패킷 객체</param>
		/// <returns>정의되지 않은 패킷 타입인 경우 false 반환</returns>
		static bool ReleasePacket(BasePacket* pPacket);


	private:
		/// <summary>
		/// 패킷 디스패처용 스레드
		/// </summary>
		static void LogicThreadProc(void* Args);

		/// <summary>
		/// LogicThreadProc 스레드 활동 여부를 설정. false가 되면 LogicThreadProc 스레드를 종료한다.
		/// </summary>
		static bool m_bLogicEnd;

		/// <summary>
		/// LogicThreadProc 스레드 개수
		/// </summary>
		int m_nLogicThreadCnt;
		int m_logicThreadGroupId;
	};
}
