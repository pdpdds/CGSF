#pragma once

class IXMLStreamWriter;
class IXMLStreamReader;

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

	void Serialize(IXMLStreamWriter* out) const;
	void Deserialize(IXMLStreamReader* in);

protected:
	

private:
	_EngineConfig m_ConfigureInfo;
};



