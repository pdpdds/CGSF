#pragma once
#include "StringConversion.h"

class SFXMLStreamWriter : public IXMLStreamWriter
{
	TiXmlDocument* pXmlDoc;

	std::list<TiXmlNode*> pNodeList;

	void writeStartDocument() 
	{
		// xml 도큐먼트에 Header을 추가한다.
		// 이건 xml 에 인코딩 헤더를 추가하는 루틴.
		if(!pXmlDoc)
			return;

		pXmlDoc->InsertDeclaration(L"1.0", L"");
	}

	void writeEndDocument() 
	{
		pNodeList.clear();

		// 현재 xml 도큐먼트를 안쓴다.
		if(!pXmlDoc)
			return;

		pXmlDoc->SaveFile();
	}

public:
	SFXMLStreamWriter(const WCHAR* xmlFile)  : pXmlDoc(NULL)
	{
		pXmlDoc = new TiXmlDocument(xmlFile);

		writeStartDocument();
	}

	~SFXMLStreamWriter() 
	{
		writeEndDocument();

		if(pXmlDoc)
		{
			delete pXmlDoc;
			pXmlDoc = NULL;
		}
	}

	static IXMLStreamWriter* CreateXmlStreamWriter(const WCHAR* xmlFile)
	{
		return new SFXMLStreamWriter(xmlFile);
	}

	void NewNode(const WCHAR* name)
	{
		if(pNodeList.empty())
			pNodeList.push_back(pXmlDoc->InsertEndChild(TiXmlElement(name)));
		else
			pNodeList.push_back(pNodeList.back()->InsertEndChild(TiXmlElement(name)));
	}

	void Begin(const WCHAR* type)
	{
		SetAttribute(L"type", type, 0);
	}

	void End()
	{
		pNodeList.pop_back();
	}

	void SetAttribute(const WCHAR* name, const WCHAR* value, int depthtype = 1)
	{
		pNodeList.back()->ToElement()->SetAttribute(name, value, depthtype);
	}

	void Write(const WCHAR* name, signed short value)
	{
		NewNode(name);

		SetAttribute(L"type", L"int16");
		SetAttribute(L"value", StringConversion::To<std::wstring>(value).c_str());

		End();
	}

	void Write(const WCHAR* name, unsigned short value)
	{
		NewNode(name);

		SetAttribute(L"type", L"uint16");
		SetAttribute(L"value", StringConversion::To<std::wstring>(value).c_str());

		End();
	}

	void Write(const WCHAR* name, int value)
	{
		NewNode(name);

		SetAttribute(L"type", L"int32");
		SetAttribute(L"value", StringConversion::To<std::wstring>(value).c_str());

		End();
	}

	void Write(const WCHAR* name, unsigned int value)
	{
		NewNode(name);

		SetAttribute(L"type", L"uint32");
		SetAttribute(L"value", StringConversion::To<std::wstring>(value).c_str());

		End();
	}

	void Write(const WCHAR* name, long value)
	{
		NewNode(name);

		SetAttribute(L"type", L"long");
		SetAttribute(L"value", StringConversion::To<std::wstring>(value).c_str());

		End();
	}

	void Write(const WCHAR* name, unsigned long value)
	{
		NewNode(name);

		SetAttribute(L"type", L"ulong");
		SetAttribute(L"value", StringConversion::To<std::wstring>(value).c_str());

		End();
	}

	void Write(const WCHAR* name, double value)
	{
		NewNode(name);

		SetAttribute(L"type", L"double");
		SetAttribute(L"value", StringConversion::To<std::wstring>(value).c_str());

		End();
	}

	void Write(const WCHAR* name, float value)
	{
		NewNode(name);

		SetAttribute(L"type", L"float");
		SetAttribute(L"value", StringConversion::To<std::wstring>(value).c_str());

		End();
	}

	void Write(const WCHAR* name, const CHAR* buffer)
	{
		NewNode(name);

		SetAttribute(L"type", L"string");
		SetAttribute(L"value", StringConversion::ToString(buffer).c_str());

		End();
	}

	void Write(const WCHAR* name, const WCHAR* buffer)
	{
		NewNode(name);

		SetAttribute(L"type", L"wstring");
		SetAttribute(L"value", StringConversion::ToString(buffer).c_str());

		End();
	}

	void Write(const WCHAR* name, const ISerializable& value)
	{
		NewNode(name);

		value.Serialize(*this);

		End();
	}
};