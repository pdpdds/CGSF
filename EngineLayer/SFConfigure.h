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
	std::wstring engineName;	
	std::wstring serverIP;
	unsigned short serverPort;
	unsigned short maxAccept;	
	
	tag_EngineConfig()
	{
		Clear();
	}

	void Clear()
	{
		serverPort = 25251;
		maxAccept = 5000;
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
		out.Write(L"EngineName",m_ConfigureInfo.engineName);
		out.Write(L"ServerIP",m_ConfigureInfo.serverIP);
		out.Write(L"ServerPort",m_ConfigureInfo.serverPort);		
		out.Write(L"MaxAccept", m_ConfigureInfo.maxAccept);
	}

	void Deserialize(IXMLStreamReader& in) {
		in.Begin(L"Struct");
		in.Read(L"EngineName", m_ConfigureInfo.engineName);
		in.Read(L"ServerIP", m_ConfigureInfo.serverIP);
		in.Read(L"ServerPort", m_ConfigureInfo.serverPort);
		in.Read(L"MaxAccept", m_ConfigureInfo.maxAccept);
	}

protected:
	

private:
	_EngineConfig m_ConfigureInfo;
};



