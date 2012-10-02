#pragma once
#include "IXMLStreamReader.h"
#include "tinyxml.h"
#include "StringConversion.h"

class SFXMLStreamReader : public IXMLStreamReader
{
	TiXmlDocument* pXmlDoc;

	std::list<TiXmlNode*> pNodeList;
	TiXmlNode*			  pNodeLast;

public:
	SFXMLStreamReader(const WCHAR* xmlFile)  : pXmlDoc(NULL), pNodeLast(NULL)
	{
		pXmlDoc = new TiXmlDocument(xmlFile);

		if(pXmlDoc)
			pXmlDoc->LoadFile();
	}

	~SFXMLStreamReader() 
	{
		if(pXmlDoc)
		{
			delete pXmlDoc;
			pXmlDoc = NULL;
		}
	}

	static IXMLStreamReader* CreateXmlStreamReader(const WCHAR* xmlFile)
	{
		return new SFXMLStreamReader(xmlFile);
	}

	BOOL Exists(const WCHAR* name) const 
	{
		TiXmlNode* pRoot = pNodeList.back();
		if(!pRoot)
			return FALSE;

		TiXmlNode *pNode = pRoot->FirstChild(name);
		if(!pNode)
			return FALSE;

		const WCHAR* nameStr = (pNode->ToElement())->Attribute(L"value");
		if(nameStr == NULL)
			return FALSE;

		if(wcslen(nameStr) == 0)
			return FALSE;

		return TRUE;
	}

	size_t GetCount(const WCHAR* name) const 
	{
		TiXmlNode* pRoot = pNodeList.back();
		if(!pRoot)
			return 0;

		int iCnt = 0;
		for (TiXmlNode *childNode = pRoot->FirstChild(name); childNode ;	childNode = childNode->NextSibling(name) )
			iCnt++;

		return iCnt;
	}

	size_t GetCount() const 
	{
		TiXmlNode* pRoot = pNodeList.back();
		if(!pRoot)
			return 0;

		int iCnt = 0;
		for (TiXmlNode *childNode = pRoot->FirstChild(); childNode ;	childNode = childNode->NextSibling() )
			iCnt++;

		return iCnt;
	}

	TiXmlNode* GetNode(const WCHAR* name)
	{
		if(pNodeList.empty())
			return pXmlDoc->FirstChild(name);

		return pNodeList.back()->FirstChild(name);
	}

	BOOL FindNode(const WCHAR* name)
	{
		if(pNodeList.empty())
			pNodeList.push_back(pXmlDoc->FirstChild(name));
		else
		{
			TiXmlNode* pNode = pNodeList.back();

			if(pNodeLast == NULL)
				pNodeList.push_back(pNode->FirstChild(name));
			else if(wcscmp(pNodeLast->Value(), name) == 0)
				pNodeList.push_back(pNodeLast->NextSibling(name));
			else 
				pNodeList.push_back(pNode->FirstChild(name));
		}

		return TRUE;
	}

	BOOL Begin(const WCHAR* type)
	{
		TiXmlNode* pRoot = pNodeList.back();
		if(!pRoot)
			return FALSE;

		const WCHAR* nameStr = (pRoot->ToElement())->Attribute(L"type");

		if(nameStr == NULL)
			return FALSE;

		if(wcscmp(nameStr, type) == 0)
			return TRUE;

		return FALSE;
	}

	void End()
	{
		pNodeLast = pNodeList.back();
		pNodeList.pop_back();
	}

	void Read(const WCHAR* name, signed short& value)
	{
		TiXmlNode *pNode = GetNode(name);
		if(!pNode)
		{
			value = 0;
			return;
		}

		int tvalue = 0;
		(pNode->ToElement())->Attribute(L"value", &tvalue);
		value = (signed short)tvalue;
	}

	void Read(const WCHAR* name, unsigned short& value)
	{
		TiXmlNode *pNode = GetNode(name);
		if(!pNode)
		{
			value = 0;
			return;
		}

		int tvalue = 0;
		(pNode->ToElement())->Attribute(L"value", &tvalue);
		value = (unsigned short)tvalue;
	}

	void Read(const WCHAR* name, int& value)
	{
		TiXmlNode *pNode = GetNode(name);
		if(!pNode)
		{
			value = 0;
			return;
		}

		(pNode->ToElement())->Attribute(L"value", &value);
	}

	void Read(const WCHAR* name, unsigned int& value)
	{
		TiXmlNode *pNode = GetNode(name);
		if(!pNode)
		{
			value = 0;
			return;
		}

		int tvalue = 0;
		(pNode->ToElement())->Attribute(L"value", &tvalue);
		value = (unsigned int)tvalue;
	}

	void Read(const WCHAR* name, long& value)
	{
		TiXmlNode *pNode = GetNode(name);
		if(!pNode)
		{
			value = 0;
			return;
		}

		int tvalue = 0;
		(pNode->ToElement())->Attribute(L"value", &tvalue);
		value = (unsigned long)tvalue;
	}

	void Read(const WCHAR* name, unsigned long& value)
	{
		TiXmlNode *pNode = GetNode(name);
		if(!pNode)
		{
			value = 0;
			return;
		}

		int tvalue = 0;
		(pNode->ToElement())->Attribute(L"value", &tvalue);
		value = (unsigned long)tvalue;
	}

	void Read(const WCHAR* name, double& value)
	{
		TiXmlNode *pNode = GetNode(name);
		if(!pNode)
		{
			value = 0;
			return;
		}

		(pNode->ToElement())->Attribute(L"value", &value);
	}

	void Read(const WCHAR* name, float& value)
	{
		TiXmlNode *pNode = GetNode(name);
		if(!pNode)
		{
			value = 0;
			return;
		}

		double tvalue;
		(pNode->ToElement())->Attribute(L"value", &tvalue);
		value = (float)tvalue;
	}

	void Read(const WCHAR* name, ISerializable& value) 
	{
		FindNode(name);

		value.Deserialize(*this);

		End();
	}

	unsigned int Read(const WCHAR* name, WCHAR* buffer, unsigned int size) 
	{
		TiXmlNode *pNode = GetNode(name);
		if(!pNode)
			return 0;

		const WCHAR* szvalue = (pNode->ToElement())->Attribute(L"value");
		std::wstring wstr = StringConversion::ToUnicode(szvalue);

		wcscpy(buffer, wstr.c_str());

		return wstr.size();
	}

	unsigned int Read(const WCHAR* name, CHAR* buffer, unsigned int size)
	{
		TiXmlNode *pNode = GetNode(name);
		if(!pNode)
			return 0;

		const WCHAR* szvalue = (pNode->ToElement())->Attribute(L"value");
		std::string str = StringConversion::ToASCII(szvalue);

		strcpy(buffer, str.c_str());

		return str.size();
	}

};