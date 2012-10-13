////////////////////////////////////////////////////////////////////////////////
//
// 작성자 : 이용휴
// 작성일 : 2006.11.16
// 설  명 :
//

#ifndef __PUCFG_H__
#define __PUCFG_H__

////////////////////////////////////////////////////////////////////////////////
//
class CPuCfg: public CCriticalClient
{
friend CPuCfg& GetPuCfgInstance();

private:
	CPuCfg();

public:
	virtual ~CPuCfg();
	BOOL Init();
	void Uninit();

public:
	void GetLogFileName(char achLogFile[MAX_PATH + 1]);
	void GetBackupLogFileName(char achBackupLogFile[MAX_PATH + 1]);

public:
	DWORD GetThxCommTimeout();
	DWORD GetThxCommSessionTimeout();
	DWORD GetMaxMsgLength();
	DWORD GetDefaultSndBuffLength();
	DWORD GetDefaultRcvBuffLength();

public:
	inline ULONG GetSerial() { return m_ulSerial; }
	inline void SetSerial(ULONG ulSerial) { m_ulSerial = ulSerial; }
	inline CMemPool<CUdpPacketNode>& GetPktPoolInstance() { return m_cPktPool; }
	inline CMemPool<CMemChunkNode>& GetMemChunkPoolInstance() { return m_cMemChunkPool; }

	inline LONG GetEchoTimeout() { return m_lEchoTimeout; }
	inline void SetEchoTimeout(LONG lEchoTimeout) { m_lEchoTimeout = lEchoTimeout; }

private:
	CMemPool<CUdpPacketNode> m_cPktPool; 
	CMemPool<CMemChunkNode> m_cMemChunkPool;

private:
	char m_achLogFile[MAX_PATH + 1];
	char m_achBackupLogFile[MAX_PATH + 1];

private:
	ULONG m_ulSerial;
	LONG m_lEchoTimeout;

private:
	BOOL m_bInit;
};

//
extern CPuCfg& GetPuCfgInstance();
	
#endif
