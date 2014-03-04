#ifndef DataBuffer_H
#define DataBuffer_H

#include "variver.h"

template<size_t SIZE>
class DataBufferT
{
	int			curLength;
	int			maxLength;
	char		datas[SIZE];
public:

	DataBufferT(void)
	{
		reset();
	}

	virtual ~DataBufferT(void)
	{
	}

	template<class TYPE> 
	TYPE& addTypeAndGetRefers()
	{
		int typelen = sizeof TYPE;
		if(maxLength-curLength < typelen)
			throw "dfdfdf"; // temp code
			//throw new OutOfBoundException("");

		TYPE& ret   = *(TYPE*)(datas+curLength);
		curLength += typelen;
		return ret;		
	}

	template<class TYPE> 
	int addTypeAndGetRefers( TYPE** out )
	{
		int typelen = sizeof TYPE;
		if(maxLength-curLength < typelen)
			return 1;

		*out   = (TYPE*)(datas+curLength);
		curLength += typelen;
		return 0;
	}

	const char dataAt(int index)
	{
		if( 0 > index || curLength <= index )
			throw "dfdfdf"; // temp code
			//throw new OutOfBoundException("");
		return reinterpret_cast<const char>(datas[index]);
	}
	
	void reset()
	{
		curLength = 0;
		maxLength = sizeof(datas);
		ZeroMemory(datas, maxLength);
	}

	int length()
	{
		return curLength;
	}

	int maxlength()
	{
		return maxLength;
	}
	
	void* toData()
	{ 
		return reinterpret_cast<void*>(datas);
	}

	const char* toCharArray()
	{
		return reinterpret_cast<const char*>(datas);
	}
};

class ReadOnlyDataBuffer
{
	int			curPos;
	int			maxLength;
	char*		datas;

public:

	ReadOnlyDataBuffer(char* p, int len)
	{
		datas = p;
		curPos = 0;
		maxLength = len;
	}

	~ReadOnlyDataBuffer()
	{
		datas = 0;
		curPos = 0;
		maxLength = 0;
	}

	__int64 ReadInt64()
	{
		__int64 v = 0;
		if(maxLength - curPos < sizeof(v))
			throw (int)-123;
		v = *((__int64*)(datas + curPos));
		curPos += sizeof(v);
		return v;
	}

	int ReadInt()
	{
		int v = 0;
		if(maxLength - curPos < sizeof(v))
			throw (int)-123;

		v = *((int*)(datas + curPos));
		curPos += sizeof(v);
		return v;
	}

	int ReadString(wchar_t* buffer, int max)
	{
		int bytes = 0;
		if(maxLength - curPos < sizeof(bytes))
			throw (int)-123;

		bytes = *( (int*)(datas+curPos) );
		curPos += sizeof(bytes);

		if(maxLength - curPos < bytes)
			throw (int)-123;

		if(bytes > max)
			CopyMemory(buffer, (datas+curPos), max);
		else
			CopyMemory(buffer, (datas+curPos), bytes);		
		curPos += bytes;

		return bytes;
	}

	int ReadStringByCount(wchar_t* buffer, int max)
	{
		int cnt = 0;
		if(maxLength - curPos < sizeof(cnt))
			throw (int)-123;

		cnt = *( (int*)(datas+curPos) );
		curPos += sizeof(cnt);

		if(maxLength - curPos < (cnt*2))
			throw (int)-123;

		if(cnt > max)
			CopyMemory(buffer, (datas+curPos), max*2);
		else
			CopyMemory(buffer, (datas+curPos), cnt*2);		
		curPos += (cnt*2);

		return (cnt*2);
	}

	int ReadArray(wchar_t* buffer, int cnt)
	{
		int bytes = sizeof(wchar_t) * cnt;
		if(0 >= bytes)
			throw (int)-123;

		if(maxLength - curPos < bytes)
			throw (int)-123;

		CopyMemory(buffer, (datas+curPos), bytes);
		curPos += bytes;

		return bytes;
	}

};

class DynamicDataBuffer
{	
	static const int DEFSIZE = 4096;
	int			curLength;
	int			maxLength;
	char*		datas;
public:

	DynamicDataBuffer(int size = DEFSIZE) : maxLength(size)
	{
		datas = new char[size];
		reset();
	}

	virtual ~DynamicDataBuffer(void)
	{
		delete[] datas;
	}

	template<class TYPE> 
	TYPE& addTypeAndGetRefers()
	{
		int typelen = sizeof TYPE;
		if(maxLength-curLength < typelen)
			throw "TooLong addTypeAndGetRefers"; // temp code
			//throw new OutOfBoundException("");

		TYPE& ret   = *(TYPE*)(datas+curLength);
		curLength += typelen;
		return ret;		
	}

	template<class TYPE> 
	int addTypeAndGetRefers( TYPE** out )
	{
		int typelen = sizeof TYPE;
		if(maxLength-curLength < typelen)
			return 1;

		*out   = (TYPE*)(datas+curLength);
		curLength += typelen;
		return 0;
	}

	const char dataAt(int index)
	{
		if( 0 > index || curLength <= index )
			throw "dfdfdf"; // temp code
			//throw new OutOfBoundException("");
		return (datas[index]);
	}

	int pushdata(char* data, int size)
	{
		if(maxLength-curLength < size)
			return 1;

		CopyMemory(datas+curLength, data, size);
		curLength += size;

		return 0;
	}

	int pushWString(wchar_t* data, int bytes)
	{
		//int& strlen = addTypeAndGetRefers<int>();
		//strlen = bytes;
		return pushdata((char*)data, bytes);
	}

	int pushString(wchar_t* data, int bytes)
	{
		int& strlen = addTypeAndGetRefers<int>();
		strlen = bytes;
		return pushdata((char*)data, bytes);
	}

	int pushWStringArray(wchar_t* src, int cnt)
	{
		int& strlen = addTypeAndGetRefers<int>();
		strlen = cnt;
		return pushdata((char*)src, cnt * sizeof(wchar_t));
	}

	int pushInt(int v)
	{
		int size = sizeof(v);
		if(maxLength-curLength < size)
			return 1;

		CopyMemory(datas+curLength, &v, size);
		curLength += size;

		return 0;
	}

	int pushInt64(__int64 v)
	{
		int size = sizeof(v);
		if(maxLength-curLength < size)
			throw "TooLong pushEmptyData";

		CopyMemory(datas+curLength, &v, size);
		curLength += size;

		return 0;
	}

	char* pushEmptyData(int size)
	{
		if(maxLength-curLength < size)
			throw "TooLong pushEmptyData";

		char* curp = datas+curLength;
		curLength += size;
		return curp;
	}
	
	void reset()
	{
		curLength = 0;		
		ZeroMemory(datas, maxLength);
	}

	int length()
	{
		return curLength;
	}

	int maxlength()
	{
		return maxLength;
	}
	
	void* toData()
	{ 
		return reinterpret_cast<void*>(datas);
	}

	const char* toCharArray()
	{
		return reinterpret_cast<const char*>(datas);
	}
};

class StaticDataBuffer
{	

//	static const int DEFSIZE = 4096;
	int			curLength;
	int			maxLength;
	char		datas[4096];
public:

	StaticDataBuffer() : maxLength(4096)
	{	
		reset();
	}

	virtual ~StaticDataBuffer(void)
	{	
	}

	template<class TYPE> 
	TYPE& addTypeAndGetRefers()
	{
		int typelen = sizeof TYPE;
		if(maxLength-curLength < typelen)
			throw "TooLong addTypeAndGetRefers"; // temp code
			//throw new OutOfBoundException("");

		TYPE& ret   = *(TYPE*)(datas+curLength);
		curLength += typelen;
		return ret;		
	}

	template<class TYPE> 
	int addTypeAndGetRefers( TYPE** out )
	{
		int typelen = sizeof TYPE;
		if(maxLength-curLength < typelen)
			return 1;

		*out   = (TYPE*)(datas+curLength);
		curLength += typelen;
		return 0;
	}

	const char dataAt(int index)
	{
		if( 0 > index || curLength <= index )
			throw "dfdfdf"; // temp code
			//throw new OutOfBoundException("");
		return (datas[index]);
	}

	int pushdata(char* data, int size)
	{
		if(maxLength-curLength < size)
			return 1;

		CopyMemory(datas+curLength, data, size);
		curLength += size;

		return 0;
	}

	int pushWString(wchar_t* data, int bytes)
	{
		//int& strlen = addTypeAndGetRefers<int>();
		//strlen = bytes;
		return pushdata((char*)data, bytes);
	}

	int pushString(wchar_t* data, int bytes)
	{
		int& strlen = addTypeAndGetRefers<int>();
		strlen = bytes;
		return pushdata((char*)data, bytes);
	}

	int pushInt(int v)
	{
		int size = sizeof(v);
		if(maxLength-curLength < size)
			return 1;

		CopyMemory(datas+curLength, &v, size);
		curLength += size;

		return 0;
	}

	int pushInt64(__int64 v)
	{
		int size = sizeof(v);
		if(maxLength-curLength < size)
			throw "TooLong pushEmptyData";

		CopyMemory(datas+curLength, &v, size);
		curLength += size;

		return 0;
	}

	char* pushEmptyData(int size)
	{
		if(maxLength-curLength < size)
			throw "TooLong pushEmptyData";

		char* curp = datas+curLength;
		curLength += size;
		return curp;
	}
	
	void reset()
	{
		curLength = 0;		
		ZeroMemory(datas, maxLength);
	}

	int length()
	{
		return curLength;
	}

	int maxlength()
	{
		return maxLength;
	}
	
	void* toData()
	{ 
		return reinterpret_cast<void*>(datas);
	}

	const char* toCharArray()
	{
		return reinterpret_cast<const char*>(datas);
	}
};



#endif