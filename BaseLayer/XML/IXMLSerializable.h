#pragma once

class IXMLStreamWriter;
class IXMLStreamReader;

class ISerializable 
{ 
public:	

	virtual void Serialize(IXMLStreamWriter* out) const=0;
	virtual void Deserialize(IXMLStreamReader* in)=0;
	virtual ~ISerializable() {}
};

template <typename T> class ArraySerialize : public ISerializable 
{
public:
	ArraySerialize(const T* pBuffer, unsigned int pSize) : buffer(pBuffer), size(pSize) 
	{
	}


	virtual void Serialize(IXMLStreamWriter* out) const 
	{
		(*out).Begin(L"array");
		for (unsigned int i=0; i<size; ++i) 
		{
			(*out).Write(L"item", buffer[i]);
		}
	}

	virtual void Deserialize(IXMLStreamReader*) 
	{ 
		assert (FALSE); 
	}

private:
	const T* buffer;
	unsigned int size;
};


template <typename T> class ArrayDeserialize : public ISerializable 
{
public:
	ArrayDeserialize(T* pBuffer, unsigned int pSize) : buffer(pBuffer), size(pSize) 
	{
	}

	virtual void Serialize(IXMLStreamWriter*) const 
	{
		assert(FALSE);
	}

	virtual void Deserialize(IXMLStreamReader* in) 
	{
		(*in).Begin(L"array");
		size_t fsize=in.GetCount(L"item");

		if (fsize >= size) 
		{
			if ( fsize != size) 
				throw Exception((L"static array being deserialized has smaller size on file"));
		} 
		else 
		{
			throw Exception((L"static array being deserialized has smaller size on file"));
		}

		for (unsigned int i=0; i<size; ++i) 
		{
			T value;
			in.Read(L"item",value);
			buffer[i]=value;
		}
	}

private:
	T* buffer;
	unsigned int size;
};

template <typename T> class ContainerSerialize : public ISerializable 
{
public:
	ContainerSerialize<T>(const T& pContainer) : container(pContainer) 
	{
	}

	virtual void Serialize(IXMLStreamWriter* out) const 
	{
		(*out).Begin(L"array");
		int counter=0;

		for (T::const_iterator it=container.begin(); it != container.end(); ++it, ++counter) 
		{
//20120821
			std::wstring item = L"item";
			item = item + StringConversion::ToUnicode(counter);

			(*out).Write(item.c_str(), *it);
		}
	}

	virtual void Deserialize(IXMLStreamReader*) 
	{ 
		assert (FALSE); 
	}

private:
	const T& container;
};

template <typename T> class ContainerDeserialize : public ISerializable 
{
public:
	ContainerDeserialize<T>(T& pContainer) : container(pContainer) 
	{
	}

	virtual void Serialize(IXMLStreamWriter*) const 
	{ 
		assert (FALSE); 
	}

	virtual void Deserialize(IXMLStreamReader* in) 
	{ 
		(*in).Begin(L"array");
		//size_t size=in.GetCount(L"item");
		size_t size=(*in).GetCount();

		for (size_t i=0; i<size; ++i) 
		{
			T::value_type value;
			std::wstring item = L"item";
			item = item + StringConversion::ToUnicode(i);
			(*in).Read(item.c_str(),value);
			container.push_back(value);
		}
	}

private:
	T& container;
};

template <typename T1, typename T2> class Pair 
{
public:
	Pair<T1,T2>(const T1& key, const T2& value) : Value(key,value) 
	{
	}

	Pair<T1,T2>() 
	{
	}

	virtual void Serialize(IXMLStreamWriter* out) const 
	{
		(*out).Begin(L"pair");
		(*out).Write(L"key", Value.first);
		(*out).Write(L"value", Value.second);

	}

	virtual void Deserialize(IXMLStreamReader* in) 
	{
		(*in).Begin(L"pair");
		(*in).Read(L"key", Value.first);
		(*in).Read(L"value", Value.second);
	}

public:
	std::pair<T1,T2> Value;
};

template <typename T> class KeyedContainerSerialize : public ISerializable 
{
public:
	KeyedContainerSerialize<T>(const T& pContainer) : container(pContainer) 
	{
	}

	virtual void Serialize(IXMLStreamWriter* out) const 
	{
		(*out).Begin(L"associative_array");
		int counter=0;

		for (T::const_iterator it=container.begin(); it != container.end(); ++it, ++counter) 
		{
			(*out).Write(L"item", Pair<T::key_type, T::mapped_type>(it->first, it->second));
		}
	}

	virtual void Deserialize(IXMLStreamReader*)
	{ 
		assert (FALSE); 
	}

private:
	const T& container;
};

template <typename T> class KeyedContainerDeserialize : public ISerializable 
{
public:
	KeyedContainerDeserialize<T>(T& pContainer) : container(pContainer) 
	{
	}

	virtual void Serialize(IXMLStreamWriter*) const 
	{ 
		assert (FALSE); 
	}

	virtual void Deserialize(IXMLStreamReader* in) 
	{
		(*in).Begin(L"associative_array");
		size_t size=in.GetCount(L"item");

		for (size_t i=0; i<size; ++i) 
		{
			Pair<T::key_type,T::mapped_type> pair;
			(*in).Read(L"item", pair);
			container.insert(pair.Value);
		}

	}

private:
	T& container;
};

template <typename T> struct WriterWrapper : ISerializable 
{
public:		
	WriterWrapper<T>(T& value) : Value(value) 
	{
	}

	virtual void Serialize(IXMLStreamWriter* out) const 
	{ 
		Value.Serialize(out); 
	}

	virtual void Deserialize(IXMLStreamReader* ) 
	{ 
		assert(FALSE);
	}

public:
	T& Value;
};

template <typename T> struct ReaderWrapper : ISerializable 
{
public:		
	ReaderWrapper<T>(T& value) : Value(value) 
	{
	}

	virtual void Serialize(IXMLStreamWriter*) const 
	{
		assert(FALSE); 
	}

	virtual void Deserialize(IXMLStreamReader* in) 
	{
		Value.Deserialize(in);
	}

public:
	T& Value;
};