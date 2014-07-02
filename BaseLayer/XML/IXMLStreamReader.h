#pragma once

#include <string>
#include <map>
#include <list>
#include <vector>

#include "IXMLSerializable.h"

#pragma warning( push )
#pragma warning( disable:4512 ) 
#pragma warning( disable:4189 )
#pragma warning( disable:4267 )
#pragma warning( disable:4100 )

class IXMLStreamReader
{
public:
	virtual ~IXMLStreamReader() {}

	virtual BOOL Exists(const WCHAR* name) const=0;
	virtual size_t GetCount(const WCHAR* name) const=0;
	virtual size_t GetCount() const=0;
	virtual BOOL FindNode(const WCHAR* name)=0;
	virtual BOOL Begin(const WCHAR* type)=0;
	virtual void End()=0;
	virtual void Read(const WCHAR* name, signed short& value)=0;
	virtual void Read(const WCHAR* name, unsigned short& value)=0;
	virtual void Read(const WCHAR* name, int& value)=0;
	virtual void Read(const WCHAR* name, unsigned int& value)=0;
	virtual void Read(const WCHAR* name, long& value)=0;
	virtual void Read(const WCHAR* name, unsigned long& value)=0;
	virtual void Read(const WCHAR* name, double& value)=0;
	virtual void Read(const WCHAR* name, float& value)=0;
	virtual unsigned int Read(const WCHAR* name, WCHAR* buffer, unsigned int size)=0;
	virtual unsigned int Read(const WCHAR* name, CHAR* buffer, unsigned int size)=0;
	virtual void Read(const WCHAR* name, ISerializable& value)=0;
	virtual void Read(const WCHAR* name, std::string& value)
	{
		CHAR tmp[MAXSTRING];
		int readed=Read(name,tmp,MAXSTRING);
		assert(readed < MAXSTRING);
		value=tmp;
	}

	virtual void Read(const WCHAR* name, std::wstring& value)
	{
		WCHAR tmp[MAXSTRING];
		int readed=Read(name,tmp,MAXSTRING);
		assert(readed < MAXSTRING);
		value=tmp;
	}

	template <typename T> void Read(const WCHAR* name,T& value) 
	{
		ReaderWrapper<T> helper(value);

		Read(name,static_cast<ISerializable&>(helper));
	}

	template <typename T> void ReadArray(const WCHAR* name,T* array, unsigned int size) 
	{
		if ( GetType() == Type::Flat) 
		{
			PrivateFlat::ArrayDeserialize<T> helper(array,size);
			Read(name,helper);
		} 
		else 
		{
			ArrayDeserialize<T> helper(array,size);
			Read(name,helper);
		}
	}

	template <typename T> void Read(const WCHAR* name, std::vector<T>& value) 
	{ 
		readContainer(name,value);	
	}

	template <typename T> void Read(const WCHAR* name, std::list<T>& value) 
	{ 
		readContainer(name,value);	
	}

	template <typename T1, typename T2> void Read(const WCHAR* name, std::map<T1,T2>& value) 
	{ 
		readKeyedContainer(name,value); 
	}

	template <typename T> void Read(const WCHAR* name, T& value, const T& defaultValue) 
	{
		if (Exists(name)) 
		{
			Read(name,value);
		} 
		else 
		{
			value=defaultValue;
		}
	}

private:
	enum 
	{ 
		MAXSTRING = 1024 /// 최대 String 길이
	};

	template <typename T> void readContainer(const WCHAR* name, T& container) 
	{
		ContainerDeserialize<T> helper(container);
		Read(name,helper);
	}

	template <typename T> void readKeyedContainer(const WCHAR* name, T& container) 
	{
		KeyedContainerDeserialize<T> helper(container);
		Read(name,helper);
	}
};

#pragma warning( pop )
