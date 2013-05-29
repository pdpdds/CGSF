#pragma once

#include "DataTypeConvertor.h"

namespace json
{
	class Object;
	class Array;
}

class JsonObjectNode;

//////////////////////////////////////////////////////////////////////////
// JsonArrayNode
//////////////////////////////////////////////////////////////////////////

class JsonArrayNode
{
public:
	JsonArrayNode();
	JsonArrayNode(const json::Array& obj);
	JsonArrayNode(const JsonArrayNode& other);
	virtual ~JsonArrayNode();

public:
	JsonArrayNode & operator = (const JsonArrayNode & other);
	bool operator == (const JsonArrayNode& object) const;

public:
	void Insert(JsonObjectNode& node);
	size_t Size() const;
	void Clear();
	JsonObjectNode GetObjectNode(int index);

	tstring ToString() const;
	void Parse(const char* content, size_t len);
	void Parse(const tstring& content);

protected:
	json::Array* JsonArrayNode::GetRawNode() const;

private:
	json::Array* m_jsonArray;

	friend class JsonObjectNode;
};

//////////////////////////////////////////////////////////////////////////
// JsonObjectNode
//////////////////////////////////////////////////////////////////////////

class JsonObjectNode
{
public:
	JsonObjectNode();
	JsonObjectNode(const json::Object& obj);
	JsonObjectNode(const JsonObjectNode& other);
	~JsonObjectNode();

public:
	JsonObjectNode & operator = (const JsonObjectNode & other);
	bool operator == (const JsonObjectNode& object) const;

public:
	template<typename T>
	void Add(const char* key, T paramValue)
	{
		const int MaxStrLen = 128;
		char strValue[MaxStrLen] = {0,};
		Datatype::ToString(strValue, paramValue, MaxStrLen);
		AddImpl(key, strValue);
	}

	template<>
	void Add(const char* key, JsonObjectNode* node) 
	{ 
		AppendObjectNode(key, node);
	}

	template<>
	void Add(const char* key, JsonArrayNode* node) 
	{ 
		AppendArrayNode(key, node);
	}

	template<typename T>
	T		GetValue(const char* key) const
	{
		tstring result = GetValueImpl(key);
		T returnValue;
		Datatype::ToOwnValue(result.c_str(), returnValue);
		return returnValue;
	}

	template<>
	JsonObjectNode GetValue(const char* key) const
	{
		return GetObjectNode(key);
	}

	template<>
	JsonArrayNode GetValue(const char* key) const
	{
		return GetArrayNode(key);
	}

	tstring ToString() const;
	unsigned int Parse(const char* content, size_t len);
	unsigned int Parse(const tstring& content);
	
protected:
	void AddImpl(const char* key, const tstring& value);
	tstring GetValueImpl(const char* key) const;
	void AppendObjectNode(const char* key, JsonObjectNode* childNode);
	void AppendArrayNode(const char* key, JsonArrayNode* childNode);
	JsonObjectNode GetObjectNode(const char* key) const;
	JsonArrayNode GetArrayNode(const char* key) const;

	json::Object* GetRawNode() const;

private:
	json::Object* m_jsonObj;
	
	friend class JsonArrayNode;
};

