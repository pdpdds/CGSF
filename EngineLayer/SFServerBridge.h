#pragma once


class SFServerBridge
{

public:
	SFServerBridge(_ServerInfo& info);
	~SFServerBridge();

	_ServerInfo& GetServerInfo(){ return m_serverInfo; }

private:
	_ServerInfo m_serverInfo;
};

