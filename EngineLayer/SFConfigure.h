#pragma once
#include <xml/tinyxml.h>
#include <xml/IXMLStreamReader.h>
#include <xml/IXMLStreamWriter.h>
#include <xml/SFXMLStreamWriter.h>
#include <xml/SFXMLStreamReader.h>

//////////////////////////////////////////////////////////////
//Shouter Structure
//////////////////////////////////////////////////////////////
typedef struct tag_EngineConfig
{
	std::wstring EngineName;
	std::wstring P2PModuleName;
	std::wstring ServerIP;
	unsigned short ServerPort;
	std::wstring PacketProtocol;
	std::wstring HostName;
	std::list<unsigned int> TimerList;
	std::wstring LogDirectory;
	
	tag_EngineConfig()
	{
		Clear();
	}

	void Clear()
	{
		ServerPort = 0;
		TimerList.clear();
	}

}_EngineConfig;

class SFConfigure
{
public:
	SFConfigure(void);
	virtual ~SFConfigure(void);

	BOOL Initialize();

	BOOL Read(const WCHAR* pFileName);
	BOOL Write(const WCHAR* pFileName);

	_EngineConfig* GetConfigureInfo(){return &m_ConfigureInfo;}

	void Serialize(IXMLStreamWriter& out) const
	{
		out.Begin(L"Struct");
		out.Write(L"EngineName",m_ConfigureInfo.EngineName);
		out.Write(L"ServerIP",m_ConfigureInfo.ServerIP);
		out.Write(L"ServerPort",m_ConfigureInfo.ServerPort);
		out.Write(L"HostName",m_ConfigureInfo.HostName);
		out.Write(L"LogDirectory",m_ConfigureInfo.LogDirectory);
	}

	void Deserialize(IXMLStreamReader& in) {
		in.Begin(L"Struct");
		in.Read(L"EngineName",m_ConfigureInfo.EngineName);
		in.Read(L"ServerIP",m_ConfigureInfo.ServerIP);
		in.Read(L"ServerPort",m_ConfigureInfo.ServerPort);
		in.Read(L"HostName",m_ConfigureInfo.HostName);
		in.Read(L"LogDirectory",m_ConfigureInfo.LogDirectory);
	}

protected:
	

private:
	_EngineConfig m_ConfigureInfo;
};



