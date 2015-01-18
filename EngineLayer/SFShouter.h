#pragma once
#include <list>

class IXMLStreamWriter;
class IXMLStreamReader;

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

	void Serialize(IXMLStreamWriter* pOut) const;
	void Deserialize(IXMLStreamReader* in);

protected:
	

private:
	_ShouterInfo m_ShouterInfo;
};



