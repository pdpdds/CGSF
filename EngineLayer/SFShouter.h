#pragma once
#include "tinyxml.h"
#include "IXMLStreamReader.h"
#include "IXMLStreamWriter.h"
#include "SFXMLStreamWriter.h"
#include "SFXMLStreamReader.h"

//////////////////////////////////////////////////////////////
//Shouter Structure
//////////////////////////////////////////////////////////////
typedef struct tag_ShouterInfo
{
	std::wstring ShouterTitle;
	int StartTime;
	int	RepeatCount;
	int MessageInterval;
	int SentenceInterval;
	std::list<std::wstring> SentenceList;

	tag_ShouterInfo()
	{
		Clear();
	}

	void Clear()
	{
		StartTime = 0;
		RepeatCount = 1;
		MessageInterval = 3000; //ms
		SentenceInterval = 10000; //ms
		SentenceList.clear();
	}

}_ShouterInfo;

class SFShouter
{
public:
	SFShouter(void);
	virtual ~SFShouter(void);

	BOOL Initialize();

	BOOL Read(const WCHAR* pFileName);
	BOOL Write(const WCHAR* pFileName);

	_ShouterInfo* GetShouterInfo(){return &m_ShouterInfo;}

	void Serialize(IXMLStreamWriter& out) const
	{
		out.Begin(L"Struct");
		out.Write(L"ShouterTitle",m_ShouterInfo.ShouterTitle);
		out.Write(L"StartTime",m_ShouterInfo.StartTime);
		out.Write(L"RepeatCount",m_ShouterInfo.RepeatCount);
		out.Write(L"MessageInterval",m_ShouterInfo.MessageInterval);
		out.Write(L"SentenceInterval",m_ShouterInfo.SentenceInterval);
		out.Write(L"SentenceList",m_ShouterInfo.SentenceList);
	}

	void Deserialize(IXMLStreamReader& in) {
		in.Begin(L"Struct");
		in.Read(L"ShouterTitle",m_ShouterInfo.ShouterTitle);
		in.Read(L"StartTime",m_ShouterInfo.StartTime);
		in.Read(L"RepeatCount",m_ShouterInfo.RepeatCount);
		in.Read(L"MessageInterval",m_ShouterInfo.MessageInterval);
		in.Read(L"SentenceInterval",m_ShouterInfo.SentenceInterval);
		in.Read(L"SentenceList",m_ShouterInfo.SentenceList);
	}

protected:
	

private:
	_ShouterInfo m_ShouterInfo;
};



